package com.aqnichol.videoproxy;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.ArrayList;

public class ClientHandler implements Runnable {

	private static ArrayList<ClientHandler> handlers = new ArrayList<ClientHandler>();
	private static int clientIdIncrement = 0;
	
	private int clientId;
	private Socket socket;
	
	private DataInputStream inputReader;
	private DataOutputStream outputWriter;
	
	private int width;
	private int height;
	
	public ClientHandler(Socket socket) throws IOException {
		this.socket = socket;
		this.clientId = clientIdIncrement++;
		
		InputStream input = socket.getInputStream();
		OutputStream output = socket.getOutputStream();
		this.inputReader = new DataInputStream(input);
		this.outputWriter = new DataOutputStream(output);
	}
	
	public int getClientId() {
		return clientId;
	}
	
	public int getWidth() {
		return width;
	}
	
	public int getHeight() {
		return height;
	}
	
	/**
	 * Respond to client requests and packets
	 */
	public void run() {
		try {
			// accept video dimensions here and broadcast them
			this.width = readInteger();
			this.height = readInteger();
			System.out.println("Client established: " + clientId);
			synchronized (handlers) {
				for (int i = 0; i < handlers.size(); i++) {
					handlers.get(i).writeInteger(1);
					handlers.get(i).writeInteger(clientId);
					handlers.get(i).writeInteger(width);
					handlers.get(i).writeInteger(height);
					this.writeInteger(1);
					this.writeInteger(handlers.get(i).getClientId());
					this.writeInteger(handlers.get(i).getWidth());
					this.writeInteger(handlers.get(i).getHeight());
				}
				handlers.add(this);
			}
			// we are now ready for our main loop
			while (true) {
				this.frameAwaitLoop();
			}
		} catch (Exception e) {
			System.out.println("Connection terminated: " + clientId);
			synchronized (handlers) {
				if (handlers.contains(this)) {
					handlers.remove(this);
				}
			}
			try {
				this.inputReader.close();
				this.outputWriter.close();
				this.socket.close();
			} catch (IOException e1) {
				e1.printStackTrace();
			}
		}
	}
	
	public void frameAwaitLoop() throws IOException {
		int dataLength = this.readInteger();
		byte[] data = new byte[dataLength];
		this.inputReader.readFully(data);
		System.out.println("Forwarding packets");
		synchronized (handlers) {
			for (int i = 0; i < handlers.size(); i++) {
				if (handlers.get(i).equals(this)) continue;
				handlers.get(i).writeInteger(2);
				handlers.get(i).writeInteger(clientId);
				handlers.get(i).writeInteger(width);
				handlers.get(i).writeInteger(height);
				handlers.get(i).writeInteger(dataLength);
				handlers.get(i).writeData(data);
			}
		}
	}
	
	public void writeData(byte[] buffer) {
		synchronized (outputWriter) {
			try {
				this.outputWriter.write(buffer);
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}
	
	public void writeInteger(int number) {
		byte[] buffer = new byte[4];
		buffer[0] = (byte)((number >> 24) & 0xff);
		buffer[1] = (byte)((number >> 16) & 0xff);
		buffer[2] = (byte)((number >> 8) & 0xff);
		buffer[3] = (byte)(number & 0xff);
		this.writeData(buffer);
	}
	
	private int readInteger() throws IOException {
		byte[] buffer = new byte[4];
		this.inputReader.readFully(buffer);
		int number = 0;
		number |= (buffer[3] & 0xff);
		number |= ((buffer[2] & 0xff) << 8);
		number |= ((buffer[1] & 0xff) << 16);
		number |= ((buffer[0] & 0xff) << 24);
		return number;
	}
	
}
