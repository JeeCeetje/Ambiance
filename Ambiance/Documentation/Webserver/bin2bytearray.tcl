proc dumpFile { fileName } {

    # Open the file, and set up to process it in binary mode.
    set f [open $fileName r]
    fconfigure $f -translation binary -encoding binary -buffering full -buffersize 16384

    while { 1 } {

        #  Read 16 bytes from the file.
        set s [read $f 16]

        # Convert the data to hex
        binary scan $s H* hex

        # Convert the hex to pairs of hex digits
        regsub -all -- {..} $hex {0x&, } hex

        # Puts the output
        puts $hex

        # Stop if we've reached end of file
        if { [string length $s] == 0 } {
            break
        }
    }

    # When we're done, close the file.
    close $f
    return
}


dumpFile "favicon_2.ico"
