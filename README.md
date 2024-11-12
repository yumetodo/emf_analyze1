# emf_analyze1

A minimum analyzer of EMF/EMF+ image.

## sample output

### EMF file

exported form Adobe Illustrator.

```cmd
>emf_analyze1.exe a.emf
device units: [0,1,188:0,249]
Picture Frame: [0,37,125:0,7,781]
version: 65,536
description: Adobe Systems
```

exported from Inkscape.

```cmd
>emf_analyze1.exe b.emf
device units: [0,9259:0,2078]
Picture Frame: [0,19599:0,4399]
version: 65536
description: Inkscape 1.3.2 (091e20e, 2023-11-25, custom) Img_Biz_Logo.emf
```

### EMF+ file

exported from Microsoft Powerpoint.

```cmd
>emf_analyze1.exe c.emf
device units: [0,738:0,162]
Picture Frame: [-3,3126:-3,687]
version: 65536
description:
EMF+ header::
GraphicsVersion: 2
EmfPlusFlags: 0
LogicalDpi: [258,258]
```

## ref

- [電波とどいた？ - 2008/10c Diary](https://ruriko.denpa.org/200810c.html)
- [c# - DPI for EMF files - Stack Overflow](https://stackoverflow.com/questions/2553300/dpi-for-emf-files)
