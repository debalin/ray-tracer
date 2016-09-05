## Read me!

Following points are to be noted for the execution of this program (built with Visual C++ in Visual Studio 2012) - 

1. This program supports multiple light sources, which need to be kept in "lights.txt" in the format "loc_x loc_y loc_z col_r col_g col_b amb diff spec".  

2. This program supports change-able window widths and heights, which need to be written in "window.txt" in the format "window_width window_height".

3. Triangle face normals are calculated by default. Vertex normals can also be calculated if not given, but this information (true/false) has to be provided in the "smoothing.txt" in the format "smooth 1_or_0". If it's 1, then vertex normals will be calculated, otherwise not. Thus, smooth shading in the absence of explicit vertex normals can be switched on or off. This on/off feature has been kept mainly because of the cube.obj whose lighting behaves weirdly on the application of smooth shading. All other OBJs work fine.

4. This program first parses and calculates normals, then does ray casting, then computes the color of the corresponding pixel with Blinn-Phong illumination - ambient, diffuse, specular and shadows are calculated. 

5. The console debug output shows the time taken for each step - parsing, ray casting and illumination calculation. 

6. Many quirks of reading OBJ/MTL files are covered (such as faces having n triangles along with vn's), but probably not all.

7. The input OBJ file has to be renamed to "input.obj" in the files/ folder. MTL file can be kept as it is. Sample OBJs and MTLs that I have tested my program with are kept in the files/ folder. The input.obj is the cube.obj at the beginning with two lights specified in the lights.txt.

8. Didn't get time to provide comments in code. But variables and data structures are self explanatory (from the names).

9. All required files are kept in the "files\" folder inside both the "Debug" and main solution directory. "freeglut.lib" and "freeglut.dll" are provided in both these directories, so that no issue occurs while running them. The .EXE file is present in the debug folder as usual. It should work just by double clicking it. 

10. Some sample outputs have been provided in the "sample_outputs" directory. The output of Al had come from the back side as its front is facing the positive z axis. I had to temporarily shift my eye location and interface window to the front to get its front side. However, my final code doesn't expose a way to do this through any file. The default value is [0, 0, -2] for the eye and an interface window parallel to the XY plane at z = -1. You can change it in the code by altering the eyeLocation.z and windowz variables. 

11. The maximum abs value for x and y and (-z) (-z means I don't care how big a positive z is, because of my eye location and interface window) are checked for in the vertices list and then all of them (x, y and z) are divided with the value found, to attempt to bring them inside the frustrum. This is probably not a smart way, just a workaround to get things rendered on the window. The front clipping plane matters (with respect to clipping) but the back clipping isn't implemented following discussions with Prof. Watson. BTW, Al takes around 30-35 minutes to render on a 400x400 window. So it might take some time before you can see it!

12. The compareNoCase function was taken from a stackoverflow page. Although the implementation is very trivial, I had got hold of a utility function which I could reuse, so used that. Other than that, some glm functions have been used.
