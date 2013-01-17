package com.aqnichol.processingnet;

import java.io.IOException;

import processing.core.PApplet;
import processing.core.PImage;

public class NetworkEncoder extends NetworkCoder {

	private int width;
	private int height;

	/**
	 * A networked H.264 encoding session
	 * @param applet The current applet
	 * @param host The host of the encode server
	 * @param port The port through which to connect to the encode server
	 * @param width The width of the images which will be encoded
	 * @param height The height of the images which will be encoded
	 * @throws IOException Thrown if the connection cannot be established.
	 */
	public NetworkEncoder(PApplet applet, String host, int port, int width, int height) throws IOException {
		super(applet, host, port);
		this.width = width;
		this.height = height;
		this.writeInteger(width);
		this.writeInteger(height);
		this.read(4); // version number
	}

	/**
	 * Encode an image to an H.264 frame
	 * @param image The image to encode
	 * @return An encoded byte arary
	 * @throws IOException Thrown when communication with the encode server fails
	 */
	public byte[] encodeImage(PImage image) throws IOException {
		byte[] data = new byte[width * height * 3];
		image.loadPixels();
		int[] colors = image.pixels;

		for (int y = 0; y < image.height; y++) {
			for (int x = 0; x < image.width; x++) {
				int index = x + (image.width * y);
				data[index * 3] = (byte)myParent.red(colors[index]);
				data[1 + index * 3] = (byte)myParent.green(colors[index]);
				data[2 + index * 3] = (byte)myParent.blue(colors[index]);
			}
		}

		this.write(data);
		int length = this.readInteger();
		System.out.println("Encoded data length: " + length);
		return this.read(length);
	}

}
