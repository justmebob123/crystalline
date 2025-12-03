/**
 * sphere_visualization_3d_correct.c
 * 
 * CORRECT 3D Kissing Spheres Visualization
 * 
 * This implements the TRUE structure:
 * - 1 central control sphere (Node 0) containing the abacus
 * - 12 worker spheres at icosahedron vertices
 * - Spheres "kiss" (touch) at shared memory points
 * - Concentric rings inside control sphere (Babylonian: 12, 60, 60, 100)
 * - Clock faces embedded on control sphere surface
 * - 12 radial lines showing 12-fold symmetry
 * - Hyper-dimensional structure with quadratic mirroring
 */

#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>
#include "../include/prime_float_math.h"

// 3D vector operations
typedef struct {
    float x, y, z;
} Vec3;

Vec3 vec3_add(Vec3 a, Vec3 b) {
    return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

Vec3 vec3_scale(Vec3 v, float s) {
    return (Vec3){v.x * s, v.y * s, v.z * s};
}

float vec3_length(Vec3 v) {
    return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
}

Vec3 vec3_normalize(Vec3 v) {
    float len = vec3_length(v);
    if (len < 0.0001f) return (Vec3){0, 0, 0};
    return vec3_scale(v, 1.0f / len);
}

// Rotate vector around X axis
Vec3 rotate_x(Vec3 v, float angle) {
    float c = prime_cosf(angle);
    float s = prime_sinf(angle);
    return (Vec3){
        v.x,
        v.y * c - v.z * s,
        v.y * s + v.z * c
    };
}

// Rotate vector around Y axis
Vec3 rotate_y(Vec3 v, float angle) {
    float c = prime_cosf(angle);
    float s = prime_sinf(angle);
    return (Vec3){
        v.x * c + v.z * s,
        v.y,
        -v.x * s + v.z * c
    };
}

// Project 3D point to 2D screen with perspective
void project_3d(Vec3 p, float rot_x, float rot_y, int center_x, int center_y, 
                int* screen_x, int* screen_y, float* depth) {
    // Apply rotations
    Vec3 rotated = rotate_x(p, rot_x);
    rotated = rotate_y(rotated, rot_y);
    
    // Perspective projection
    float camera_dist = 2.5f;
    float perspective = camera_dist / (camera_dist + rotated.z / 100.0f);
    
    *screen_x = center_x + (int)(rotated.x * perspective);
    *screen_y = center_y - (int)(rotated.y * perspective);
    *depth = rotated.z;
}

// Draw a 3D circle (clock face)
void draw_3d_circle(SDL_Renderer* renderer, Vec3 center, float radius, 
                    Vec3 normal, float rot_x, float rot_y, 
                    int screen_cx, int screen_cy, SDL_Color color) {
    // Generate circle points
    const int num_points = 72;
    
    // Find two perpendicular vectors in the plane
    Vec3 u, v;
    if (fabsf(normal.z) < 0.9f) {
        u = vec3_normalize((Vec3){-normal.y, normal.x, 0});
    } else {
        u = vec3_normalize((Vec3){0, -normal.z, normal.y});
    }
    v = vec3_normalize((Vec3){
        normal.y * u.z - normal.z * u.y,
        normal.z * u.x - normal.x * u.z,
        normal.x * u.y - normal.y * u.x
    });
    
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    for (int i = 0; i < num_points; i++) {
        float angle1 = (i * 2.0f * PRIME_PI) / num_points;
        float angle2 = ((i + 1) * 2.0f * PRIME_PI) / num_points;
        
        Vec3 p1 = vec3_add(center, vec3_add(
            vec3_scale(u, prime_cosf(angle1) * radius),
            vec3_scale(v, prime_sinf(angle1) * radius)
        ));
        
        Vec3 p2 = vec3_add(center, vec3_add(
            vec3_scale(u, prime_cosf(angle2) * radius),
            vec3_scale(v, prime_sinf(angle2) * radius)
        ));
        
        int x1, y1, x2, y2;
        float d1, d2;
        project_3d(p1, rot_x, rot_y, screen_cx, screen_cy, &x1, &y1, &d1);
        project_3d(p2, rot_x, rot_y, screen_cx, screen_cy, &x2, &y2, &d2);
        
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
}

// Draw clock tick marks
void draw_clock_ticks(SDL_Renderer* renderer, Vec3 center, float radius,
                      Vec3 normal, float rot_x, float rot_y,
                      int screen_cx, int screen_cy, SDL_Color color) {
    // Find perpendicular vectors
    Vec3 u, v;
    if (fabsf(normal.z) < 0.9f) {
        u = vec3_normalize((Vec3){-normal.y, normal.x, 0});
    } else {
        u = vec3_normalize((Vec3){0, -normal.z, normal.y});
    }
    v = vec3_normalize((Vec3){
        normal.y * u.z - normal.z * u.y,
        normal.z * u.x - normal.x * u.z,
        normal.x * u.y - normal.y * u.x
    });
    
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    // Draw 12 tick marks
    for (int i = 0; i < 12; i++) {
        float angle = (i * 2.0f * PRIME_PI) / 12.0f;
        
        Vec3 tick_pos = vec3_add(center, vec3_add(
            vec3_scale(u, prime_cosf(angle) * radius),
            vec3_scale(v, prime_sinf(angle) * radius)
        ));
        
        int tx, ty;
        float td;
        project_3d(tick_pos, rot_x, rot_y, screen_cx, screen_cy, &tx, &ty, &td);
        
        // Draw tick mark (small filled circle)
        for (int dx = -2; dx <= 2; dx++) {
            for (int dy = -2; dy <= 2; dy++) {
                if (dx*dx + dy*dy <= 4) {
                    SDL_RenderDrawPoint(renderer, tx + dx, ty + dy);
                }
            }
        }
    }
}

// Draw a 3D sphere (wireframe)
void draw_3d_sphere_wireframe(SDL_Renderer* renderer, Vec3 center, float radius,
                               float rot_x, float rot_y, int screen_cx, int screen_cy,
                               SDL_Color color, int alpha) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, alpha);
    
    // Draw latitude circles
    for (int lat = -2; lat <= 2; lat++) {
        float y = (lat / 2.0f) * radius;
        float r = sqrtf(radius * radius - y * y);
        if (r < 0.1f) continue;
        
        const int num_points = 36;
        for (int i = 0; i < num_points; i++) {
            float angle1 = (i * 2.0f * PRIME_PI) / num_points;
            float angle2 = ((i + 1) * 2.0f * PRIME_PI) / num_points;
            
            Vec3 p1 = vec3_add(center, (Vec3){
                prime_cosf(angle1) * r, y, prime_sinf(angle1) * r
            });
            Vec3 p2 = vec3_add(center, (Vec3){
                prime_cosf(angle2) * r, y, prime_sinf(angle2) * r
            });
            
            int x1, y1, x2, y2;
            float d1, d2;
            project_3d(p1, rot_x, rot_y, screen_cx, screen_cy, &x1, &y1, &d1);
            project_3d(p2, rot_x, rot_y, screen_cx, screen_cy, &x2, &y2, &d2);
            
            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }
    }
    
    // Draw longitude circles
    for (int lon = 0; lon < 6; lon++) {
        float angle_offset = (lon * PRIME_PI) / 6.0f;
        
        const int num_points = 36;
        for (int i = 0; i < num_points; i++) {
            float angle1 = (i * 2.0f * PRIME_PI) / num_points;
            float angle2 = ((i + 1) * 2.0f * PRIME_PI) / num_points;
            
            Vec3 p1 = vec3_add(center, (Vec3){
                prime_cosf(angle_offset) * prime_sinf(angle1) * radius,
                prime_cosf(angle1) * radius,
                prime_sinf(angle_offset) * prime_sinf(angle1) * radius
            });
            Vec3 p2 = vec3_add(center, (Vec3){
                prime_cosf(angle_offset) * prime_sinf(angle2) * radius,
                prime_cosf(angle2) * radius,
                prime_sinf(angle_offset) * prime_sinf(angle2) * radius
            });
            
            int x1, y1, x2, y2;
            float d1, d2;
            project_3d(p1, rot_x, rot_y, screen_cx, screen_cy, &x1, &y1, &d1);
            project_3d(p2, rot_x, rot_y, screen_cx, screen_cy, &x2, &y2, &d2);
            
            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }
    }
}

// Main 3D kissing spheres visualization
void draw_kissing_spheres_3d_correct(SDL_Renderer* renderer, int center_x, int center_y,
                                     int base_radius, float rot_x, float rot_y) {
    // Golden ratio for icosahedron
    const float phi = (1.0f + sqrtf(5.0f)) / 2.0f;
    
    // 12 icosahedron vertices (normalized)
    Vec3 vertices[12] = {
        {0, 1, phi},   {0, -1, phi},   {0, 1, -phi},  {0, -1, -phi},
        {1, phi, 0},   {-1, phi, 0},   {1, -phi, 0},  {-1, -phi, 0},
        {phi, 0, 1},   {-phi, 0, 1},   {phi, 0, -1},  {-phi, 0, -1}
    };
    
    // Normalize vertices
    for (int i = 0; i < 12; i++) {
        vertices[i] = vec3_normalize(vertices[i]);
    }
    
    // Sphere sizes
    float control_radius = base_radius * 0.6f;  // Central control sphere
    float worker_radius = base_radius * 0.15f;  // Worker spheres
    
    Vec3 control_center = {0, 0, 0};
    
    // === STEP 1: Draw concentric rings inside control sphere ===
    float ring_radii[4] = {0.25f, 0.5f, 0.75f, 1.0f};  // Babylonian: 12, 60, 60, 100
    SDL_Color ring_colors[4] = {
        {100, 150, 255, 80},  // Inner - blue
        {150, 200, 255, 60},
        {200, 220, 255, 40},
        {220, 230, 255, 20}   // Outer - light blue
    };
    
    for (int ring = 0; ring < 4; ring++) {
        float r = control_radius * ring_radii[ring];
        draw_3d_sphere_wireframe(renderer, control_center, r, rot_x, rot_y,
                                  center_x, center_y, ring_colors[ring], 60);
    }
    
    // === STEP 2: Draw control sphere (semi-transparent) ===
    SDL_Color control_color = {200, 180, 100, 255};  // Gold
    draw_3d_sphere_wireframe(renderer, control_center, control_radius, rot_x, rot_y,
                              center_x, center_y, control_color, 100);
    
    // === STEP 3: Draw clock faces embedded on control sphere ===
    // XY plane (horizontal)
    draw_3d_circle(renderer, control_center, control_radius * 0.95f, (Vec3){0, 0, 1},
                   rot_x, rot_y, center_x, center_y, (SDL_Color){80, 120, 160, 255});
    draw_clock_ticks(renderer, control_center, control_radius * 0.95f, (Vec3){0, 0, 1},
                     rot_x, rot_y, center_x, center_y, (SDL_Color){120, 160, 200, 255});
    
    // XZ plane (vertical front)
    draw_3d_circle(renderer, control_center, control_radius * 0.95f, (Vec3){0, 1, 0},
                   rot_x, rot_y, center_x, center_y, (SDL_Color){80, 120, 160, 255});
    draw_clock_ticks(renderer, control_center, control_radius * 0.95f, (Vec3){0, 1, 0},
                     rot_x, rot_y, center_x, center_y, (SDL_Color){120, 160, 200, 255});
    
    // YZ plane (vertical side)
    draw_3d_circle(renderer, control_center, control_radius * 0.95f, (Vec3){1, 0, 0},
                   rot_x, rot_y, center_x, center_y, (SDL_Color){80, 120, 160, 255});
    draw_clock_ticks(renderer, control_center, control_radius * 0.95f, (Vec3){1, 0, 0},
                     rot_x, rot_y, center_x, center_y, (SDL_Color){120, 160, 200, 255});
    
    // === STEP 4: Draw 12 radial lines ===
    SDL_Color radial_colors[12] = {
        {255, 100, 100, 255}, {100, 255, 100, 255}, {100, 100, 255, 255},
        {255, 255, 100, 255}, {255, 100, 255, 255}, {100, 255, 255, 255},
        {255, 150, 100, 255}, {150, 255, 100, 255}, {100, 150, 255, 255},
        {255, 100, 150, 255}, {100, 255, 150, 255}, {150, 100, 255, 255}
    };
    
    for (int i = 0; i < 12; i++) {
        Vec3 endpoint = vec3_scale(vertices[i], control_radius + worker_radius);
        
        int x1, y1, x2, y2;
        float d1, d2;
        project_3d(control_center, rot_x, rot_y, center_x, center_y, &x1, &y1, &d1);
        project_3d(endpoint, rot_x, rot_y, center_x, center_y, &x2, &y2, &d2);
        
        SDL_SetRenderDrawColor(renderer, radial_colors[i].r, radial_colors[i].g, 
                               radial_colors[i].b, 128);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
    
    // === STEP 5: Draw 12 worker spheres at icosahedron vertices ===
    for (int i = 0; i < 12; i++) {
        Vec3 worker_pos = vec3_scale(vertices[i], control_radius);
        
        // Draw worker sphere
        SDL_Color worker_color = radial_colors[i];
        draw_3d_sphere_wireframe(renderer, worker_pos, worker_radius, rot_x, rot_y,
                                  center_x, center_y, worker_color, 255);
        
        // === STEP 6: Draw kissing point (highlight) ===
        int kx, ky;
        float kd;
        project_3d(worker_pos, rot_x, rot_y, center_x, center_y, &kx, &ky, &kd);
        
        // Bright highlight at kissing point
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (int dx = -3; dx <= 3; dx++) {
            for (int dy = -3; dy <= 3; dy++) {
                if (dx*dx + dy*dy <= 9) {
                    SDL_RenderDrawPoint(renderer, kx + dx, ky + dy);
                }
            }
        }
    }
}