use std::io::{Read, Write};
use std::net::{TcpStream, ToSocketAddrs};
use std::time::Duration;

use serde::Deserialize;

use crate::{Error, Result};

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct ServerAddress {
    pub host: String,
    pub port: u16,
}

impl ServerAddress {
    pub fn parse(value: &str) -> Result<Self> {
        let value = value.trim();
        if value.is_empty() {
            return Err(Error::Invalid("server address is empty".into()));
        }
        let (host, port) = if let Some((host, port)) = value.rsplit_once(':') {
            if host.contains(':') && !host.starts_with('[') {
                (value, 25565)
            } else {
                let port = port
                    .parse::<u16>()
                    .map_err(|_| Error::Invalid(format!("invalid server port in '{value}'")))?;
                (host.trim_matches(['[', ']']), port)
            }
        } else {
            (value, 25565)
        };
        if host.is_empty() {
            return Err(Error::Invalid("server host is empty".into()));
        }
        if host.chars().any(|ch| ch.is_whitespace() || ch == '/') {
            return Err(Error::Invalid(format!("invalid server host '{host}'")));
        }
        Ok(Self {
            host: host.to_string(),
            port,
        })
    }

    pub fn display(&self) -> String {
        if self.host.contains(':') {
            format!("[{}]:{}", self.host, self.port)
        } else {
            format!("{}:{}", self.host, self.port)
        }
    }
}

#[derive(Clone, Debug, Default, PartialEq, Eq)]
pub struct ServerStatus {
    pub version_name: String,
    pub protocol: i64,
    pub players_online: i64,
    pub players_max: i64,
    pub motd: String,
    pub latency_ms: u128,
    pub favicon: Option<String>,
}

#[derive(Deserialize)]
struct StatusResponse {
    #[serde(default)]
    version: StatusVersion,
    #[serde(default)]
    players: StatusPlayers,
    #[serde(default)]
    description: StatusDescription,
    #[serde(default)]
    favicon: Option<String>,
}

#[derive(Default, Deserialize)]
struct StatusVersion {
    #[serde(default)]
    name: String,
    #[serde(default)]
    protocol: i64,
}

#[derive(Default, Deserialize)]
struct StatusPlayers {
    #[serde(default)]
    online: i64,
    #[serde(default)]
    max: i64,
}

#[derive(Default, Deserialize)]
#[serde(untagged)]
enum StatusDescription {
    Text(String),
    Object {
        #[serde(default)]
        text: String,
    },
    #[default]
    Empty,
}

impl StatusDescription {
    fn text(&self) -> String {
        match self {
            StatusDescription::Text(text) => text.clone(),
            StatusDescription::Object { text } => text.clone(),
            StatusDescription::Empty => String::new(),
        }
    }
}

pub fn ping(address: &ServerAddress, timeout: Duration) -> Result<ServerStatus> {
    let started = std::time::Instant::now();
    let socket = format!("{}:{}", address.host, address.port);
    let target = socket
        .to_socket_addrs()
        .map_err(|error| Error::Invalid(format!("cannot resolve {socket}: {error}")))?
        .next()
        .ok_or_else(|| Error::NotFound(socket.clone()))?;
    let mut stream = TcpStream::connect_timeout(&target, timeout)
        .map_err(|error| Error::Invalid(format!("cannot connect to {socket}: {error}")))?;
    stream.set_read_timeout(Some(timeout))?;
    stream.set_write_timeout(Some(timeout))?;
    stream.set_nodelay(true)?;

    let mut handshake = Vec::new();
    write_varint(&mut handshake, 0);
    write_varint(&mut handshake, 765);
    write_string(&mut handshake, &address.host);
    handshake.extend_from_slice(&address.port.to_be_bytes());
    write_varint(&mut handshake, 1);
    write_packet(&mut stream, &handshake)?;
    write_packet(&mut stream, &[0])?;
    let packet = read_packet(&mut stream)?;
    let mut cursor = 0;
    let _id = read_varint(&packet, &mut cursor)?;
    let json_len = read_varint(&packet, &mut cursor)? as usize;
    if cursor + json_len > packet.len() {
        return Err(Error::Invalid("server status response is truncated".into()));
    }
    let response: StatusResponse = serde_json::from_slice(&packet[cursor..cursor + json_len])?;
    Ok(ServerStatus {
        version_name: response.version.name,
        protocol: response.version.protocol,
        players_online: response.players.online,
        players_max: response.players.max,
        motd: clean_motd(&response.description.text()),
        latency_ms: started.elapsed().as_millis(),
        favicon: response.favicon,
    })
}

fn clean_motd(text: &str) -> String {
    let mut out = String::with_capacity(text.len());
    let mut chars = text.chars().peekable();
    while let Some(ch) = chars.next() {
        if ch == '§' {
            chars.next();
        } else {
            out.push(ch);
        }
    }
    out.trim().to_string()
}

fn write_packet(stream: &mut TcpStream, payload: &[u8]) -> Result<()> {
    let mut packet = Vec::new();
    write_varint(&mut packet, payload.len() as i32);
    packet.extend_from_slice(payload);
    stream.write_all(&packet)?;
    Ok(())
}

fn read_packet(stream: &mut TcpStream) -> Result<Vec<u8>> {
    let length = read_varint_stream(stream)?;
    if length <= 0 || length > 2 * 1024 * 1024 {
        return Err(Error::Invalid(format!(
            "server status packet length {length} is invalid"
        )));
    }
    let mut payload = vec![0u8; length as usize];
    stream.read_exact(&mut payload)?;
    Ok(payload)
}

fn write_varint(output: &mut Vec<u8>, value: i32) {
    let mut value = value as u32;
    loop {
        if value & !0x7f == 0 {
            output.push(value as u8);
            return;
        }
        output.push(((value & 0x7f) | 0x80) as u8);
        value >>= 7;
    }
}

fn write_string(output: &mut Vec<u8>, value: &str) {
    write_varint(output, value.len() as i32);
    output.extend_from_slice(value.as_bytes());
}

fn read_varint(bytes: &[u8], cursor: &mut usize) -> Result<i32> {
    let mut result = 0u32;
    let mut shift = 0;
    loop {
        let byte = *bytes
            .get(*cursor)
            .ok_or_else(|| Error::Invalid("varint is truncated".into()))?;
        *cursor += 1;
        result |= ((byte & 0x7f) as u32) << shift;
        if byte & 0x80 == 0 {
            return Ok(result as i32);
        }
        shift += 7;
        if shift >= 35 {
            return Err(Error::Invalid("varint is too long".into()));
        }
    }
}

fn read_varint_stream(stream: &mut TcpStream) -> Result<i32> {
    let mut result = 0u32;
    let mut shift = 0;
    loop {
        let mut byte = [0u8; 1];
        stream.read_exact(&mut byte)?;
        result |= ((byte[0] & 0x7f) as u32) << shift;
        if byte[0] & 0x80 == 0 {
            return Ok(result as i32);
        }
        shift += 7;
        if shift >= 35 {
            return Err(Error::Invalid("varint is too long".into()));
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn parses_host_and_default_port() {
        let address = ServerAddress::parse("play.example.org").unwrap();
        assert_eq!(address.host, "play.example.org");
        assert_eq!(address.port, 25565);
    }

    #[test]
    fn parses_explicit_port() {
        let address = ServerAddress::parse("localhost:25566").unwrap();
        assert_eq!(address.display(), "localhost:25566");
    }

    #[test]
    fn rejects_invalid_addresses() {
        assert!(ServerAddress::parse("").is_err());
        assert!(ServerAddress::parse("host:not-a-port").is_err());
    }

    #[test]
    fn motd_codes_are_removed() {
        assert_eq!(clean_motd("§aHello §lWorld"), "Hello World");
    }
}
