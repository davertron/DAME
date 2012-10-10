DAME
====

Multi-Emulator Frontend

Requirements:
* [Cinder 0.8.2](http://libcinder.org/download/)
* [Boost 1.44](http://www.boost.org/)

After you've downloaded boost, you'll need to copy the **boost_1_44_0/boost_1_44_0/boost** directory to **cinder/boost/boost** (if you don't have a boost directory in cinder yet, just create one). You'll also need to get the program_options binary libs for windows as well. You can download them using the [boost pro downloader](http://www.boostpro.com/download/). Download and run it, select your version of Visual Studio (I used 10.0), and then tick off all of the "variants" (they aren't that big and I don't really know the difference). It will have all of the possible libs ticked off; untick everything except "Boost ProgramOptions" and hit next. Set your download directory, and let it download the files. Then you'll need to copy **libboost_program_options-vc100-mt-s-1_44** and **libboost_program_options-vc100-mt-sgd-1_44** to your cinder/lib/msw folder. You'll notice that we're only copying two files out of the 12 or so that got downloaded; the other ones are the other "variants" we ticked off before.

After all that you should be good to go.