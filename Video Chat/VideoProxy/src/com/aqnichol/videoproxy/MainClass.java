package com.aqnichol.videoproxy;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

public class MainClass {

	public static void main(String[] args) {
		
		/*
		if (args.length == 0) {
			System.err.println("Usage: java videoproxy.jar port");
			return;
		}
		
		int port = Integer.parseInt(args[0]);*/
		int port = 5904;
		ServerSocket server = null;
		try {
			server = new ServerSocket(port);
		} catch (IOException ex) {
			ex.printStackTrace();
			return;
		}
		// continuously accept new client connections
		try {
			while (true) {
				Socket clientSocket = null;
				clientSocket = server.accept();
				handleClient(clientSocket);
			}
		} catch (IOException ex) {
			ex.printStackTrace();
		}
	}
	
	public static void handleClient(Socket client) throws IOException {
		ClientHandler handler = new ClientHandler(client);
		new Thread(handler).start();
	}
	
}
