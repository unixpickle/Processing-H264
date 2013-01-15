package com.aqnichol.processingnet;

import java.io.IOException;

import processing.core.PApplet;
import processing.core.PImage;

public class NetworkDecoder extends NetworkCoder {

	private int width;
	private int height;

	/**
	 * Instantiate a networked decoder session.
	 * @param applet The current applet
	 * @param host The host of the decode server process
	 * @param port The port on which to connect
	 * @param width The width of the images which will be decoded
	 * @param height The height of the images which will be decoded
	 * @throws IOException Thrown when the connection could not be created
	 */
	public NetworkDecoder(PApplet applet, String host, int port, int width, int height) throws IOException {
		super(applet, host, port);
		this.width = width;
		this.height = height;
		this.writeInteger(width);
		this.writeInteger(height);
		this.read(4); // version number
	}

	/**
	 * Decode a complete H.264 frame
	 * @param data The frame data
	 * @return The decoded image.
	 * @throws IOException Thrown when communication with the decode server fails
	 */
	public PImage decodeH264Data(byte[] data) throws IOException {
		this.writeInteger(data.length);
		this.write(data);
		int length = this.width * this.height * 3;
		byte[] buffer = this.read(length);
		PImage image = new PImage(width, height);
		image.loadPixels();
		int[] pixelBuffer = image.pixels;
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				int index = x + (y * width);
				int red = (int)(buffer[index * 3] & 0xff);
				int green = (int)(buffer[index * 3 + 1] & 0xff);
				int blue = (int)(buffer[index * 3 + 2] & 0xff);
				pixelBuffer[index] = myParent.color(red, green, blue);
			}
		}
		image.updatePixels();
		return image;
	}

}
