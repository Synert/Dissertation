## Procedural Solar System (*Not No Man's Sky*)

### About
My 3rd year university dissertation. The goal was to generate a solar system with varied planets that could be explored in realtime, using dynamic level-of-detail meshes.

### Build instructions
This will only run on x64 Windows machines, and has only been tested on Windows 10.
The project is set up to use relative paths, and should work straight away.
Everything necessary is included in the debug/release folders, although debug mode's pretty slow.

### Example planets

![](http://www.synert.co.uk/images/blog/planets.png)

### Libraries used
- [libnoise](http://libnoise.sourceforge.net/)
- [64 bit libnoise build](https://github.com/eldernos/LibNoise64)
- [AntTweakBar](http://anttweakbar.sourceforge.net/doc/)

### Additional credits
- [Rastertek tutorials](http://www.rastertek.com/), used to create the backend for rendering
- [stack overflow](https://stackoverflow.com/questions/9296059/read-pixel-value-in-bmp-file), BMP reading
- [Microsoft](https://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.geometric.xmvector3normalize(v=vs.85).aspx), robust normalize