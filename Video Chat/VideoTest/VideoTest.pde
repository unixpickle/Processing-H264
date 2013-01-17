import processing.net.*;
import processing.video.*;
import com.aqnichol.processingnet.*;
import java.io.IOException;

Capture video;

String proxyAddress = "127.0.0.1";
int proxyPort = 5904;
BasicVideoProxyConnection proxy = null;
int lastUpdate = 0;

void setup() {
  String[] cameras = Capture.list();
  if (cameras.length == 0) {
    video = null;
  } else {
    video = new Capture(this, cameras[0]);
    video.start();
  }
  size(1280, 720);
  frameRate(10);
}

void draw() {
  if (video.available()) {
    video.read();
    if (proxy == null) {
      println("video is available " + video.width + " height " + height);
      try {
        proxy = new BasicVideoProxyConnection(this, proxyAddress, proxyPort, video.width, video.height, "127.0.0.1", 1337, "127.0.0.1", 1338);
        proxy.start();
      } catch (IOException e) {
        e.printStackTrace();
      }
      lastUpdate = millis();
      return;
    }
    if (millis() > lastUpdate + 200) {
      System.gc();
      lastUpdate = millis();
      try {
        if (proxy.getIsPaired()) {
          proxy.writeFrame(video);
        }
      } catch (IOException e) {
        e.printStackTrace();
      }
      println("time to send frame: " + (millis() - lastUpdate));
    }
  }
  if (proxy == null) return;
  boolean hasDrawn = false;
  if (proxy.getIsPaired()) {
    if (proxy.getImage() != null) {
      image(proxy.getImage(), 0, 0);
      g.removeCache(proxy.getImage());
      hasDrawn = true;
    }
  }
  if (!hasDrawn) {
    image(video, 0, 0);
  }
}

