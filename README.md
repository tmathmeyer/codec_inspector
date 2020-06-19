# Codec Inspector
Have you ever wanted to get detailed frame-by-frame information about a video
file and found ffprobe to be too unspecific? do you need to see literal bitstream
data like this: http://downloads.webmproject.org/docs/vp9/vp9-bitstream_superframe-and-uncompressed-header_v1.0.pdf
this tool kinda does that.

## Installing & Building
1. You need a chromium checkout, the whole thing. Including all the ninja & gn stuff.
2. You need python3 to run the scripts. 
3. Clone this repo and run `install_this_project.py [chromium_source_directory]`
4. Head over to the root of your chromium source tree.
5. Run `./media/tools/codec_inspector/build.py`
6. Get the binary `./out/codec_inspector/codec_inspecor_backend`

## Running
1. Run the binary.
2. type `file [path_to_media_file]`
3. type `track [number]` (loading a file will present you with availible tracks)
4. type `frame` or `f` to show information about the next frame
5. type `skip [number]` to skip that many frames
6. type `help` to show a help menu
7. type `quit` to quit

## Future work
 - Make a better UI for this, like some hacked together frontend or webserver
   or something so that the frames can be expanded and looked at easily
 - More commands
 - Check it into chrome and expose it in devtools - lol.