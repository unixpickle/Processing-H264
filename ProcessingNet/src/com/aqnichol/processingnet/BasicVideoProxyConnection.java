package com.aqnichol.processingnet;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.net.UnknownHostException;

import processing.core.PApplet;
import processing.core.PImage;

public class BasicVideoProxyConnection extends NetworkCoder implements Runnable {

	PApplet applet;
	private NetworkEncoder encoder;
	private NetworkDecoder decoder;
	int pairedClientId;
	boolean isPaired = false;
	
	String decodeHost = null;
	int decodePort = 0;
	
	private PImage image = null;
	private Object imageLock;
	
	public BasicVideoProxyConnection(PApplet mainApplet, String host, int proxyPort, int width, int height, String encoderHost, int encoderPort, String decoderHost, int decoderPort) throws IOException {
		super(mainApplet, host, proxyPort);
		applet = mainApplet;
		encoder = new NetworkEncoder(mainApplet, encoderHost, encoderPort, width, height);
		decodeHost = decoderHost;
		decodePort = decoderPort;
		imageLock = new Object();
		this.writeInteger(width);
		this.writeInteger(height);
	}
	
	public void start() {
		new Thread(this).start();
	}
	
	public void close() throws IOException {
		try {
			if (decoder != null) decoder.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
		try {
			encoder.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
		super.close();
	}
	
	public void writeFrame(PImage image) throws IOException {
		byte[] h264Data = encoder.encodeImage(image);
		this.writeInteger(h264Data.length);
		this.write(h264Data);
	}
	
	public boolean getIsPaired() {
		return isPaired;
	}
	
	public PImage getImage() {
		synchronized (imageLock) {
			return image;
		}
	}

	public void run() {
		try {
			while (true) {
				int packetId = this.readInteger();
				if (packetId == 1) {
					if (!isPaired) {
						pairedClientId = this.readInteger();
						int clientWidth = this.readInteger();
						int clientHeight = this.readInteger();
						decoder = new NetworkDecoder(applet, decodeHost, decodePort, clientWidth, clientHeight);
						isPaired = true;
					} else {
						this.read(12);
					}
				} else if (packetId == 2) {
					int clientId = this.readInteger();
					int frameWidth = this.readInteger();
					int frameHeight = this.readInteger();
					int frameLength = this.readInteger();
					byte[] data = this.read(frameLength);
					if (clientId == pairedClientId && isPaired) {
						PImage anImage = decoder.decodeH264Data(data);
						synchronized (imageLock) {
							image = anImage;
						}
					}
					data = null;
				}
			}
		} catch (IOException exception) {
			exception.printStackTrace();
		}
	}
	
}
