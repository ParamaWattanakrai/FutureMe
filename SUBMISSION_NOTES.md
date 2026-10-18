# FUTURE ME — submission notes

## What this is
A complete, buildable Modern OpenGL (3.3 core) scene recreating the reference
photo (stairs climbing to a dark rectangular doorway against an open sky),
built entirely from **vertex data** — no external 3D model or texture file.
Lighting is a **Blinn-Phong model** (ambient + diffuse + specular) driven by
one directional light; the sky is a second, unlit full-screen shader with a
gradient and simple procedural (noise-based) clouds.

It drops straight into the course's `OpenGLStarter` project: same
`Window`/`Shader` classes, same CMake + vcpkg build, no new dependencies.

## What changed vs. the starter
- `src/Libs/Mesh.h` / `.cpp` — added `CreateMeshLit()`, a second vertex
  layout (position + normal + colour) alongside the original position-only
  `CreateMesh()`, so lighting has normals and every box can carry its own
  colour.
- `Shaders/scene.vert` / `scene.frag` — the Phong-lit shader for the
  staircase, platform and doorway.
- `Shaders/sky.vert` / `sky.frag` — the unlit sky-gradient + cloud shader.
- `src/main.cpp` — builds all scene geometry procedurally (`AddBox()` +
  `CreateScene()` / `CreateSkyQuad()`), sets the camera/light, and renders.
  This is the file to rename for submission (see below).

## Before you submit
1. **Fill in your names/IDs.** Replace the `STUDENTID1`/`STUDENTID2`
   placeholders in this folder and inside the header comment of
   `src/main.cpp`.
2. **Rename the source file** to `Assignment2_<studentID>.cpp` as the brief
   asks (an already-renamed copy, `Assignment2_STUDENTID1_STUDENTID2.cpp`,
   is included as a template — just replace the IDs and point CMake at it,
   or simplest: keep working in `src/main.cpp` and rename that one file
   right before you zip/submit).
3. **Capture your own image.** Build and run the program (F7 then F5 in
   VS Code, as in the main `README.md`), then press **S** — it saves
   `screenshot.bmp` next to the executable. Re-save that as `.jpg`/`.png`
   (Paint, Preview, GIMP, or any online converter) named
   `Assignment2_<studentID>_<pairID>.png`, per the brief.
   `Assignment2_STUDENTID1_STUDENTID2.png` in this folder is the reference
   render produced the same way (offscreen, same code) — replace it with
   your own capture once you've adjusted the scene to taste.
4. Add every group member's name/ID to the Google Classroom submission
   comment, and credit this reference photo (stairs + dark doorway
   composition) there too, per the "References" rubric line.

## Ideas for the "Dedication" rubric line (easy, optional extensions)
- Swap the single directional light for a point light at the doorway
  mouth, so it glows faintly as if something is lit beyond it.
- Animate the sun slightly (rotate `lightDirection` over time) for a subtle
  moving-cloud-shadow effect — `sky.frag` already receives a `time` uniform.
- Add a second, smaller flight of stairs or a figure silhouette for scale.
- Texture the stair treads/platform with a subtle stone/concrete texture
  (the brief explicitly allows external textures).

## Controls
- `S` — save a screenshot (`screenshot.bmp`) of the current frame
- `Esc` / close button — quit
