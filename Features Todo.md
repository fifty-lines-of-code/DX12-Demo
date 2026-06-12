Features to implement:



1. ~~Basic Camera~~
2. ~~Per frame resources, 3 frames, to keep gpu and cpu busy at almost all times~~
3. ~~Xbox controller support to move player (cube)~~
4. ~~A plane "floor"~~
5. ~~A plane "wall"~~
6. ~~Camera rotation around player~~
7. ~~FromSoft style movement of camera behind player when player moves~~
8. ~~Interpolate and Rotate the player to camera direction (if needed) based on controller input~~
9. ~~Slow walk when controller held 50% or less, normal walk otherwise~~
10. ~~Running when player holds B (Xbox) / Circle (PS) while walking~~
11. ~~Fromsoft style back dash when player is still and presses B/Circle~~
12. ~~Refactor so that we use Vec3, Vec4, Matrix4x4. Also move Renderer inside Engine.~~
13. ~~Move polling of Xbox input to background thread~~
14. ~~Full screen support~~
15. ~~Switch between fullscreen and windowed  - esc to go window, F for fullscreen~~
16. ~~Collision detection of player with wall~~
17. ~~Optimized Collision detection using OctTree~~
18. ~~Start in fullscreen~~
19. ~~Remove references to entities in OctTree and just store indexes as our entities are stored in a flat array~~
20. ~~Chunk, ChunkSlot, ChunksManager~~
21. ~~Generate Terrain of Chunk0x0 Programmatically~~
22. ~~Create and load static height map and set heights and colors of Chunk0x0 generated in step 21~~
23. ~~Basic lighting~~
24. ~~Create and load static height map~~
25. ~~Setup pipeline and Load a single texture and apply it to player cube~~
26. Player "breathing" animation when idle
27. Walking animation
28. Running animation
29. Smoothly interpolate from breathing into movement
30. Rolling animation in the direction of controls - infinite direction dictated by controls
31. PS controller support
32. FromSoft style lockon
33. Main menu
34. Frustum Culling
35. Dynamic loading and unloading of data as player moves/crosses a boundary
36. integrate dynamic height map into dynamic chunks loader
37. implement waterfall in the heightmap
38. render waterfall
39. Load fbx models - use ue5 models
40. 
41. 

