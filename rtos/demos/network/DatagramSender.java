import java.net.*;
import java.util.*;

public class DatagramSender {
   public static void main(String argv[]) throws Exception {
	long i = 0;
      DatagramSocket sender = new DatagramSocket();
      InetAddress IP = InetAddress.getByName("192.168.121.2");
		String line=("Inviato");
		byte[] buffer = new byte[255];
		byte[] buffer1 ;
		DatagramPacket packetr;

		while ( true ) {
         //DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
         //System.out.println("In attesa di messaggi...");
         //receiver.receive(packet);
         //String message = new String(packet.getData(), packet.getOffset(), packet.getLength());
			//System.out.println("Messaggio ricevuto dall'host " + packet.getAddress() + " su porta " + packet.getPort() + ": " + message+" "+ i++);
			line="Inviato"+i;
			buffer1 = line.getBytes();
         packetr = new DatagramPacket(buffer1, buffer1.length, IP, 100);
         sender.send(packetr);
			if(i++ % 10==0)
				System.out.println("Inviato"+i);
			Thread.sleep(508,0);
	
	 //System.out.println(message+" "+ i++);
      }

   }
}



