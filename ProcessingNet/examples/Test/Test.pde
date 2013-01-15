import processing.net.*;
import com.aqnichol.processingnet.*;
import java.io.*;

void setup() {
    size(10, 10);
    try {
        NetworkEncoder enc = new NetworkEncoder(this, "127.0.0.1", 1337, 640, 480);
        NetworkDecoder dec = new NetworkDecoder(this, "127.0.0.1", 1338, 640, 480);
        PImage img = loadImage("/Users/alex/Desktop/image.jpg");
        
        // encode the data and save it to a raw file on my Desktop
        int start = millis();
        byte[] encoded = enc.encodeImage(img);
        saveBytes("/Users/alex/Desktop/save.h264", encoded);
        println("time to encode: " + (millis() - start));
        
        // re-decode the h.264 data into a PImage object.
        start = millis();
        PImage newImg = dec.decodeH264Data(encoded);
        println("time to decode: " + (millis() - start));
    } catch (IOException e) {
    }
}

void draw() {
}