/*
 * The Rust side of LibMatti (library/src/libmatti/rs).
 *
 * C is the API: the launcher, the mod loader, the early window and the OpenGL
 * renderer run first and stay in C. The game main then crosses into Rust for
 * the UI and returns to C when Rust hands control back. These declarations are
 * the only symbols the two sides share - keep them in sync with
 * library/src/libmatti/rs/src/lib.rs.
 */

#ifndef MATTICRAFT_LIBMATTI_RS_MATTI_RS_H
#define MATTICRAFT_LIBMATTI_RS_MATTI_RS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Rust: matti_rs_init. Returns 0 on success, non-zero on failure. The Rust
 * side catches panics, so it never unwinds into C. */
int matti_rs_init(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif /* MATTICRAFT_LIBMATTI_RS_MATTI_RS_H */
