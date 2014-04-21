ancient-stats
=============

This is the ancient C version of the LLR statistics and related utilities.  This is for reference only.  

This directory contains a collection of text processing code that I used to use.  That was
back in the stone age.  Back before dirt was invented and all we had were rocks to compute
with.

As such, this code looks like it should be in the stone age.  I have made a cursory attempt
to get the code to work on a modern compiler, but you should be aware that there are almost
certainly going to be surprises if you depend on this code.

You can find out more about the LLR algorithm at 

http://tdunning.blogspot.com/2008/03/surprise-and-coincidence.html

The rest of the code should have documentation here in the form of man pages.  More or less.

I am happy to answer questions about the statistical theory behind this code, but the code itself
should be considered a lost cause, rooted as it is in a time and place too far away to much matter.

to compile
=========

On Linux, do this:

    make ARCH=linux 

On other operating systems, linux may be the best bet since things have converged a bit since this
code was written.  I have verified that the code compiles on OSX using the linux option, for instance.


why are headlines in lower case?
==============

That is the way I used to write things.  Seems appropriate to do that here.
