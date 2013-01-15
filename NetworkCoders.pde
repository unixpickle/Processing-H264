/**
 * This is a deprecated file. I suggest using my ProcessingNet
 * extension instead of this for improved performance.
 */

PApplet mainApplet = this;

class NetworkCoder {
  Client client;
  
  NetworkCoder(String address, int port) {
    client = new Client(mainApplet, address, port);
  }
  
  Client getClient() {
    return client;
  }
  
  void close() {
    client.stop();
  }
  
  byte[] read_bytes(int length) {
    while (client.available() < length) {}
    byte[] buffer = new byte[length];
    for (int i = 0; i < length; i++) {
      buffer[i] = (byte)client.read();
    }
    return buffer;
  }
  
  int read_integer() {
    int number = 0;
    while (client.available() < 4) {}
    number |= int(client.read()) << 24;
    number |= int(client.read()) << 16;
    number |= int(client.read()) << 8;
    number |= int(client.read());
    return number;
  }
  
  void write_integer(int aNumber) {
    byte[] buff = new byte[4];
    buff[3] = (byte)(aNumber & 0xff);
    buff[2] = (byte)((aNumber >> 8) & 0xff);
    buff[1] = (byte)((aNumber >> 16) & 0xff);
    buff[0] = (byte)((aNumber >> 24) & 0xff);
    client.write(buff);
  }
  
}

class NetworkEncoder extends NetworkCoder {
  
  NetworkEncoder(String address, int port, int width, int height) {
    super(address, port);
    this.read_bytes(4);
    this.write_integer(width);
    this.write_integer(height);
  }
  
  byte[] h264_encode_image(PImage anImage) {
    this.getClient().write(this.encode_frame(anImage));
    int recLen = this.read_integer();
    return this.read_bytes(recLen);
  }
  
  byte[] encode_frame(PImage anImage) {
    anImage.loadPixels();
    color[] colors = anImage.pixels;
    byte[] buffer = new byte[anImage.width * anImage.height * 3];
    for (int y = 0; y < anImage.height; y++) {
      for (int x = 0; x < anImage.width; x++) {
        int index = x + (anImage.width * y);
        buffer[index * 3] = (byte)red(colors[index]);
        buffer[1 + index * 3] = (byte)green(colors[index]);
        buffer[2 + index * 3] = (byte)blue(colors[index]);
      }
    }
    return buffer;
  }
  
}

class NetworkDecoder extends NetworkCoder {

  int width;
  int height;
  
  NetworkDecoder(String address, int port, int theWidth, int theHeight) {
    super(address, port);
    width = theWidth;
    height = theHeight;
    this.read_bytes(4);
    this.write_integer(width);
    this.write_integer(height);
  }
  
  PImage h264_decode_data(byte[] buffer) {
    this.write_integer(buffer.length);
    this.getClient().write(buffer);
    int size = width * height * 3;
    return this.image_from_bytes(this.read_bytes(size));
  }
  
  PImage image_from_bytes(byte[] buffer) {
    int start = millis();
    PImage image = new PImage(width, height, RGB);
    image.loadPixels();
    color[] colors = image.pixels;
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        int index = x + (y * width);
        int red = int(buffer[index * 3]);
        int green = int(buffer[index * 3 + 1]);
        int blue = int(buffer[index * 3 + 2]);
        colors[index] = color(red, green, blue);
      }
    }
    image.updatePixels();
    println("image_from_bytes time: " + (millis() - start)); 
    return image;
  }

}
