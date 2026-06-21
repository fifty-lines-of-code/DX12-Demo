# 🎮 DX12 Real-Time Game Engine

> A custom real-time game engine built from scratch using DirectX 12, focused on explicit GPU programming, frame synchronization, and data-oriented architecture.

---

## ⚡ What This Project Is

This is a **low-level real-time engine** designed to replicate core systems found in modern game engines:

- Explicit DirectX 12 rendering pipeline
- GPU command submission architecture
- Frame synchronization (CPU/GPU parallel execution)
- Data-oriented scene and memory design
- Real-time gameplay + rendering integration

It is built to understand how real-time engines operate at the system level, not just how to use graphics APIs.

---

## 🧠 Core Engine Systems

### 🖼️ Rendering (DirectX 12)

- Multi-pass rendering pipeline
- Manual command list recording per pass
- Explicit GPU resource binding model (descriptor heaps, root signatures, PSOs)
- Resource state transitions and barrier-based synchronization model
- Explicit GPU command submission pipeline control

---

### ⏱ Frame System

- Triple-buffered frame architecture
- Fence-based CPU/GPU synchronization
- Frame-indexed resource management
- Safe parallel execution between CPU and GPU
- CPU builds frame N while GPU executes frame N-1.

---

### 🧩 Scene Architecture

- Flat array entity storage (cache-efficient design)
- Slot reuse allocation system (no fragmentation)
- Chunk-based world partitioning system
- Octree spatial structure for fast queries and culling

---

### 🎮 Input & Gameplay System

- Keyboard + Xbox controller support
- Multithreaded double-buffered input system
- Third-person movement system:
  - Walk / run / sprint scaling
  - Context-aware dash mechanics

---

### ⚔️ Collision System

- AABB-based collision detection
- Lightweight entity integration
- Real-time gameplay validation

---

### 🧪 Rendering Features

- Font rendering (quad + atlas sampling)
- Compute shader 2-pass blur
- Terrain rendering (heightmap-based)
- Phong lighting model

---

## 🎮 Controls

### Keyboard (Debug)

- **F** → Toggle fullscreen / windowed mode  
- **D** → Toggle debug UI window  

---

### Xbox Controller

- **Left Stick**
  - < 50% → slow walk  
  - &gt;= 50% → normal walk  

- **Hold B while moving** → Run  

- **Press B from idle**
  - Short intent window (~X seconds):
    - If movement intent detected → run from idle
    - Otherwise → backwards dash

> Input system resolves movement intent vs action ambiguity using a short buffering window.

---

## ⚙️ Performance Design Philosophy

- Flat memory layouts (cache-friendly design)
- Minimal pointer indirection
- Explicit resource ownership
- Deterministic frame behavior
- No hidden engine magic

> Every CPU/GPU interaction is intentional and visible.

---

## 🧱 Tech Stack

- C++ (ISO C++17)
- DirectX 12
- HLSL
- Win32 API
- Multithreaded CPU systems

---

## 📐 Engine Execution Flow (High-Level Mental Model)

> The engine is structured around explicit CPU/GPU parallelism:
>
> The CPU builds commands for the current frame while the GPU may still be executing work submitted from previous frames.
> Before reusing frame-indexed resources, the engine waits on the associated fence value to ensure the GPU has completed work referencing those resources.

### Initialization
- Initialize subsystems (camera, scene, input, rendering, spatial structures)

### Main Loop
- Poll double-buffered input system
- Update simulation systems (world, camera, gameplay)
- Perform collision detection (AABB)
- Synchronize frame resources using fences
- Execute render pass system
- Record DirectX 12 command lists (GPU submission pipeline)
- Submit work via GPU command queue
- Present frame

---

## 📚 Resources & Inspiration

This project is informed by industry-standard graphics and engine development resources:

- **Introduction to 3D Game Programming with DirectX 12 (1st Edition)** — Frank Luna  
  Core reference for DirectX 12 rendering pipeline, GPU resource management, and explicit graphics programming.

- **Game Engine Architecture (3rd Edition)** — Jason Gregory  
  Reference for large-scale engine architecture, system design, and real-time engine organization.

- **Microsoft DirectX 12 Documentation (Official API Reference)**  
  Primary reference for API behavior, resource binding, synchronization models, and low-level GPU programming details.

- Additional learning derived from graphics experimentation, GPU debugging, and iterative engine development.

---

## 📌 What This Demonstrates

- Real-time engine architecture design from first principles
- Explicit GPU submission and frame scheduling systems
- Low-level graphics API proficiency (DirectX 12)
- Barrier-based synchronization and resource lifetime management
- Data-oriented performance engineering
- Integration of rendering, gameplay, and physics systems
- Understanding of CPU/GPU parallel execution models

---

## 🧭 Current Work

- Planar reflection system (render-to-texture)
- Pre-shadow projection experiments

---

## 🚀 Future Work

- Dynamic Chunk loading 
- Shadow mapping system
- FBX skeletal animation system
- Physically Based Rendering (PBR)
- Render graph architecture
- Frustum culling (octree-driven)
- Wireframe debug rendering mode

---

## 📸 Media

> Coming soon:

- Gameplay clips
- Terrain rendering
- Debug UI overlays
- Camera movement
- Render pass breakdowns
- Architecture diagram

---

## 🧠 Why This Project Exists

Built to bridge professional software engineering experience into real-time graphics programming by implementing engine systems from scratch, focusing on:

- GPU programming models
- Engine architecture design
- Performance-critical system design