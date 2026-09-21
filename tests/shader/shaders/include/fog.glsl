float fog_distance(vec3 pos, float start, float end) {
    return clamp((length(pos) - start) / (end - start), 0.0, 1.0);
}
