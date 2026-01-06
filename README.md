**Turtle Arena** game code.

To use this you'll need the [Spearmint engine](https://github.com/clover-moe/spearmint).

  * On Windows, install [Cygwin and mingw-w64](https://github.com/clover-moe/spearmint/wiki/Compiling#windows).
  * Get the source for Spearmint and build it using `make`.
  * Get the source for this repo and build it using `make`.
  * Copy the [turtle-arena-data](https://github.com/Turtle-Arena/turtle-arena-data) to `turtle-arena-code/build/release-mingw32-x86/baseturtle/data.pk3dir`

If you put both projects in the same directory you can launch the game using;

    spearmint/build/release-mingw32-x86/spearmint_x86.exe +set fs_basepath "turtle-arena-code/build/release-mingw32-x86/" +set fs_game "baseturtle"

On Linux and OS X you'll need to put `./` before the command and substitute the correct platform and architecture (look in the build directory).

## License

ebx-code is licensed under a [modified version of the GNU GPLv3](COPYING.txt#L625) (or at your option, any later version). This is due to including code from Return to Castle Wolfenstein and Wolfenstein: Enemy Territory.

Submitted contributions must be given with permission to use as GPLv**2** (two) and any later version; unless the file is under a license besides the GPL, in which case that license applies. This allows me to potentially change the license to GPLv2 or later in the future.

## Credits

* Zack Middleton (main developer)
* Tobias Kuehnhammer (feedback / bug reports / Bot AI fixes)
* And other contributors

Spearmint contains code from;
* Quake 3 - id Software
* ioquake3 - ioquake3 contributors
* RTCW SP - Gray Matter Interactive
* RTCW MP - Nerve Software
* Wolfenstein: Enemy Territory - Splash Damage
* Tremulous - Dark Legion Development
* World of Padman - Padworld Entertainment
* [ioEF engine](http://thilo.tjps.eu/efport-progress/) - Thilo Schulz
* NetRadiant's q3map2 - Rudolf Polzer
* OpenArena - OpenArena contributors
* OpenMoHAA - OpenMoHAA contributors
* Xreal (triangle mesh collision) - Robert Beckebans
* ZEQ2-lite (cel shading) - ZEQ2 project
