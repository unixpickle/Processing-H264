package com.aqnichol.processingnet;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;

import processing.core.PApplet;

public class NetworkCoder {

	// myParent is a reference to the parent sketch
	protected PApplet myParent;

	public final static String VERSION = "1.0";
	
	private Socket socket;
	private DataOutputStream output;
	private DataInputStream input;
	
	public NetworkCoder(PApplet theParent, String address, int port) throws IOException {
		myParent = theParent;
		socket = new Socket(address, port);
		output = new DataOutputStream(socket.getOutputStream());
		input = new DataInputStream(socket.getInputStream());
	}

	public static String version() {
		return VERSION;
	}
	
	public void close() throws IOException {
		this.input.close();
		this.output.close();
		this.socket.close();
	}
	
	public void write(byte[] data) throws IOException {
		this.output.write(data);
	}
	
	public byte[] read(int length) throws IOException {
		byte[] data = new byte[length];
		this.input.readFully(data);
		return data;
	}
	
	public byte readByte() throws IOException {
		return this.input.readByte();
	}
	
	public void writeInteger(int number) throws IOException {
		byte[] buffer = new byte[4];
		buffer[0] = (byte)((number >> 24) & 0xff);
		buffer[1] = (byte)((number >> 16) & 0xff);
		buffer[2] = (byte)((number >> 8) & 0xff);
		buffer[3] = (byte)(number & 0xff);
		this.output.write(buffer);
	}
	
	public int readInteger() throws IOException {
		byte[] buffer = new byte[4];
		this.input.readFully(buffer);
		int number = 0;
		number |= (buffer[3] & 0xff);
		number |= ((buffer[2] & 0xff) << 8);
		number |= ((buffer[1] & 0xff) << 16);
		number |= ((buffer[0] & 0xff) << 24);
		return number;
	}
	
}
