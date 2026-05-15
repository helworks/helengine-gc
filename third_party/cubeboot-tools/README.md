# cubeboot-tools apploader subset

This directory vendors the minimal open-source source files required to build a GameCube `apploader.img` for Helengine packaged-disc builds.

Source provenance:

- Project: `cubeboot-tools`
- Upstream package: `cubeboot-tools-0.3`
- Author credits in upstream headers: The GameCube Linux Team, Albert Herranz
- Upstream download used for import: `https://downloads.sourceforge.net/project/gc-linux/cubeboot-tools/0.3/cubeboot-tools-0.3.tar.bz2`

Only the files needed to compile the apploader payload were copied:

- `include/dol.h`
- `include/gcm.h`
- `include/lib.h`
- `common/lib.c`
- `icons/opening.bnr`
- `mkgbi/mkgbi.c`
- `ppc/apploader/apploader.c`
- `ppc/apploader/apploader_ldscript.txt`
- `ppc/common/lib.c`
- `ppc/common/misc.S`
- `ppc/include/system.h`

Each vendored source file retains its original license header. Those headers state GPL v2 or later.
