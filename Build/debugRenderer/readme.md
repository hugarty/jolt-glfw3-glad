# Guidelines

All of the shaders here follow the following standard variable names

* `position`: Refers to the 3d positional information of a singular vertex, may be relative or absolute
* `position`: Refers to the 3d positional information of a singular vertex
* `local_to_world`: Refers to the matrix/linear transformation from R4 to R4 that maps a local position eg measured to the objects center and converts that into a position in the game world
* `world_to_camera`: Refers to the matrix/linear transformation from R4 to R4 that translates the origin of the game world to where the camera is now.
* `camera_to_clip`: Refers to the matrix/linear transformation from R4 to R4 that applies camera effects to vertices, the result of this are points in clip space.
* `passthrough_texture_position`: Refers to the variable used to pass texture positions through the vertex shader to the fragment shader as you cannot pass data directly to the fragement shader
* `texture_position`: Refers to (U/V)'s which are simply numbers from (0, 1)x(0, 1) which represent where we want to sample a texture to be applied to a triangle.

## Terminology and Explanation

`local_to_world` is a matrix which translates space to change the origin
if you have an object, and you don't want it to appear at the origin
then you can use the local_to_world matrix to store the 3d translation
EX) your character is holding a gun and you use the players position as
a 4x4 matrix which encodes a 3d translation and use that to draw the gun
relative to the players position. For fixed objects like the map, this is
usually the identity

`world_to_camera`  transforms space so that the origin is now the camera, that is
to say if the camera was located at (1, 1, 1) and then there is an object at
(2, 1, 1) then after setting the camera's position to be the origin, this point
now becomes (1, 0, 0) relative to the camera.
EX) Your character has a camera attached to them, and they move, you want your
camera view to move as well, thus by updating this matrix you can acheive that

`camera_to_clip` is a transformation based on the physical properties of our camera
we transform vertices to apply perspective. Shouldn't change in our context unless FOV
is changed.

## Entire Pipeline

A regular vertex shader then might do the following:
Given a vertex v in R3, we do CTC WTC LTM v
which is matrix multiplication and moves from inside out.

now opengl looks at the result of this composite transformation, if it lands in clip space
then the vertex will be rendered, otherwise it is outside of view.

After the last transformation is complete, all vertices are considered to be in clip space and
will be visible or not based on the above
