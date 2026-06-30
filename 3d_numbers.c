#include "raylib.h"
#include "raymath.h"
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_POINTS 1000000

typedef struct
{
    int number;
    Vector3 pos;
    float scale;
} NumberPoint;

static NumberPoint points[MAX_POINTS];
static int point_count = 0;

/* secret formula */
static Vector3
get_coord (int p)
{
    const float k = 0.05f;
    float t = (float)p * k;

    return (Vector3){ t * sinf (t) * cosf (t), t * sinf (t),
                      t * cosf (t) };
}

static bool
is_prime (int n)
{
    if (n < 2)
        return false;
    if (n == 2)
        return true;
    if (n % 2 == 0)
        return false;

    for (int d = 3; d * d <= n; d += 2)
        if (n % d == 0)
            return false;

    return true;
}

static void
add_number (int n)
{
    if (n < 0 || n > MAX_POINTS)
        return;

    if (!is_prime (n))
        return;

    for (int i = 0; i < point_count; i++)
        if (points[i].number == n)
            return;

    points[point_count++] = (NumberPoint){
        .number = n,
        .pos = get_coord (n),
        .scale = 0.0f,
    };
}

int
main (void)
{
    const int SW = 1280, SH = 720;
    InitWindow (SW, SH, "3D Number Visualizer");
    SetTargetFPS (60);

    Camera3D cam = {
        .position = { 10.0f, 10.0f, 25.0f },
        .target = { 0.0f, 5.0f, 5.0f },
        .up = { 0.0f, 1.0f, 0.0f },
        .fovy = 50.0f,
        .projection = CAMERA_PERSPECTIVE,
    };

    float yaw = -2.4f;
    float pitch = 0.4f;
    float radius = 28.0f;
    Vector3 target = cam.target;
    char input_buf[64] = { 0 };
    int input_len = 0;
    int range_buf[MAX_POINTS];
    int range_count = 0, range_idx = 0;
    bool range_mode = false;
    float anim_timer = 0.0f;

    while (!WindowShouldClose ())
    {
        float dt = GetFrameTime ();

        bool mouse_down = IsMouseButtonDown (MOUSE_BUTTON_RIGHT);
        Vector2 mdelta = GetMouseDelta ();

        if (mouse_down)
        {
            yaw -= mdelta.x * 0.004f;
            pitch -= mdelta.y * 0.004f;
            if (pitch > 1.4f)
                pitch = 1.4f;
            if (pitch < -1.4f)
                pitch = -1.4f;
        }

        float wheel = GetMouseWheelMove ();
        radius -= wheel * 1.5f;
        if (radius < 2.0f)
            radius = 2.0f;
        if (radius > 200.0f)
            radius = 200.0f;

        float move_speed = 8.0f * dt;
        Vector3 forward
            = Vector3Normalize (Vector3Subtract (target, cam.position));
        forward.y = 0;
        forward = Vector3Normalize (forward);
        Vector3 right = Vector3CrossProduct (forward, (Vector3){ 0, 1, 0 });

        if (IsKeyDown (KEY_W))
            target = Vector3Add (target, Vector3Scale (forward, move_speed));
        if (IsKeyDown (KEY_S))
            target = Vector3Add (target, Vector3Scale (forward, -move_speed));
        if (IsKeyDown (KEY_A))
            target = Vector3Add (target, Vector3Scale (right, -move_speed));
        if (IsKeyDown (KEY_D))
            target = Vector3Add (target, Vector3Scale (right, move_speed));
        if (IsKeyDown (KEY_Q))
            target.y -= move_speed;
        if (IsKeyDown (KEY_E))
            target.y += move_speed;

        cam.position.x = target.x + radius * cosf (pitch) * sinf (yaw);
        cam.position.y = target.y + radius * sinf (pitch);
        cam.position.z = target.z + radius * cosf (pitch) * cosf (yaw);
        cam.target = target;

        int ch;
        while ((ch = GetCharPressed ()) != 0)
        {
            if ((isdigit (ch) || ch == '-') && input_len < 63)
            {
                input_buf[input_len++] = (char)ch;
                input_buf[input_len] = '\0';
            }
        }
        if (IsKeyPressed (KEY_BACKSPACE) && input_len > 0)
            input_buf[--input_len] = '\0';

        if (IsKeyPressed (KEY_ENTER) && input_len > 0)
        {
            const char *dash = strchr (input_buf, '-');
            if (dash)
            {
                int a = atoi (input_buf);
                int b = atoi (dash + 1);
                if (a > b)
                {
                    int t = a;
                    a = b;
                    b = t;
                }
                range_count = 0;
                for (int i = a; i <= b && range_count < MAX_POINTS; i++)
                    range_buf[range_count++] = i;
                range_idx = 0;
                range_mode = true;
                anim_timer = 0.0f;
            }
            else
            {
                int n = atoi (input_buf);
                if (n >= 0)
                    add_number (n);
            }
            input_buf[0] = '\0';
            input_len = 0;
        }

        if (range_mode && range_idx < range_count)
        {
            anim_timer += dt;
            float delay = (range_count > 500) ? 0.01f : 0.06f;
            while (anim_timer >= delay && range_idx < range_count)
            {
                add_number (range_buf[range_idx++]);
                anim_timer -= delay;
            }
            if (range_idx >= range_count)
                range_mode = false;
        }

        for (int i = 0; i < point_count; i++)
        {
            if (points[i].scale < 1.0f)
            {
                points[i].scale += dt * 8.0f;
                if (points[i].scale > 1.0f)
                    points[i].scale = 1.0f;
            }
        }

        BeginDrawing ();
        ClearBackground ((Color){ 14, 14, 20, 255 });

        BeginMode3D (cam);

        DrawLine3D ((Vector3){ -200, 0, 0 }, (Vector3){ 200, 0, 0 },
                    (Color){ 200, 60, 60, 180 });
        DrawLine3D ((Vector3){ 0, -200, 0 }, (Vector3){ 0, 200, 0 },
                    (Color){ 60, 200, 60, 180 });
        DrawLine3D ((Vector3){ 0, 0, -200 }, (Vector3){ 0, 0, 200 },
                    (Color){ 60, 120, 255, 180 });

        for (int i = -10; i <= 10; i++)
        {
            float f = i * 2.0f;
            DrawLine3D ((Vector3){ f, 0, -20 }, (Vector3){ f, 0, 20 },
                        (Color){ 50, 50, 70, 120 });
            DrawLine3D ((Vector3){ -20, 0, f }, (Vector3){ 20, 0, f },
                        (Color){ 50, 50, 70, 120 });
        }

        for (int i = 0; i < point_count; i++)
        {
            NumberPoint *p = &points[i];
            float s = p->scale;
            if (s <= 0.01f)
                continue;

            Color col = (Color){ 80, 160, 255, 255 };

            int layer = p->number / 100;
            if (layer == 1)
                col = (Color){ 255, 160, 60, 255 };
            if (layer == 2)
                col = (Color){ 80, 220, 120, 255 };
            if (layer >= 3)
                col = (Color){ 220, 80, 220, 255 };

            DrawSphere (p->pos, 0.28f * s, col);

            Vector3 base = { p->pos.x, 0, p->pos.z };
            DrawLine3D (base, p->pos, (Color){ col.r, col.g, col.b, 100 });
        }

        EndMode3D ();

        for (int i = 0; i < point_count; i++)
        {
            NumberPoint *p = &points[i];
            if (p->scale < 0.5f)
                continue;
            Vector3 above = { p->pos.x, p->pos.y + 0.5f, p->pos.z };
            Vector2 sc = GetWorldToScreen (above, cam);
            if (sc.x < 0 || sc.x > SW || sc.y < 0 || sc.y > SH)
                continue;
            char label[16];
            snprintf (label, sizeof (label), "%d", p->number);
            int tw = MeasureText (label, 14);
            DrawText (label, (int)sc.x - tw / 2, (int)sc.y - 7, 14, WHITE);
        }

        {
            Vector2 ax = GetWorldToScreen ((Vector3){ 18, 0, 0 }, cam);
            Vector2 ay = GetWorldToScreen ((Vector3){ 0, 18, 0 }, cam);
            Vector2 az = GetWorldToScreen ((Vector3){ 0, 0, 18 }, cam);
            DrawText ("X (n/100)", (int)ax.x + 4, (int)ax.y - 8, 16,
                      (Color){ 220, 80, 80, 255 });
            DrawText ("Y (n/10)%10", (int)ay.x + 4, (int)ay.y - 8, 16,
                      (Color){ 80, 220, 80, 255 });
            DrawText ("Z (n%10)", (int)az.x + 4, (int)az.y - 8, 16,
                      (Color){ 80, 140, 255, 255 });
        }

        int bx = 20, by = SH - 56, bw = SW - 40, bh = 40;
        DrawRectangle (bx, by, bw, bh, (Color){ 24, 24, 36, 230 });
        DrawRectangleLines (bx, by, bw, bh, (Color){ 80, 160, 255, 200 });
        DrawText ("type num or massive:", bx + 8, by - 20, 15,
                  (Color){ 140, 160, 210, 255 });
        char disp[128];
        snprintf (disp, sizeof (disp), "%s%s", input_buf,
                  ((int)(GetTime () * 2) % 2 == 0) ? "|" : " ");
        DrawText (disp, bx + 12, by + 10, 22, (Color){ 80, 210, 255, 255 });

        if (range_mode || range_idx < range_count)
        {
            char prog[64];
            snprintf (prog, sizeof (prog), "adding: %d / %d", range_idx,
                      range_count);
            DrawText (prog, bx + 8, by - 40, 15,
                      (Color){ 100, 240, 140, 255 });
            float frac = range_count ? (float)range_idx / range_count : 0;
            DrawRectangle (bx, by - 46, (int)(bw * frac), 4,
                           (Color){ 80, 255, 120, 200 });
        }

        char cnt[64];
        snprintf (cnt, sizeof (cnt), "points: %d", point_count);
        DrawText (cnt, SW - 130, 12, 16, (Color){ 140, 160, 200, 255 });

        EndDrawing ();
    }

    CloseWindow ();
    return 0;
}
