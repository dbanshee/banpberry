package banshee.banpberrycontroller;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import android.content.Context;
import android.os.AsyncTask;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;

public class MediaPlayer {
	
	private Context ctx 				= null;
	private View mediaPlayerView 		= null;
	
	private static final String BBHOST 	= "192.168.15.1";
	private static final int 	PORT 	= 12345;
	
	private String intentUrl			= null;
	
	public MediaPlayer(Context ctx){
		this.ctx = ctx;
	}
	
	public void setView(View mediaPlayerView){
		this.mediaPlayerView = mediaPlayerView;
		
		Button seekRightButton = (Button) mediaPlayerView.findViewById(R.id.fforward_button);
		seekRightButton.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				seekFW();
			}
		});
		
		Button seekLeftButton = (Button) mediaPlayerView.findViewById(R.id.fbackward_button);
		seekLeftButton.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				seekBW();
			}
		});
		
		Button seekUpButton = (Button) mediaPlayerView.findViewById(R.id.nexttrack_button);
		seekUpButton.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				seekUp();
			}
		});
		
		Button seekDownButton = (Button) mediaPlayerView.findViewById(R.id.prevtrack_button);
		seekDownButton.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				seekDown();
			}
		});
		
		
		Button pauseButton = (Button) mediaPlayerView.findViewById(R.id.pause_button);
		pauseButton.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				pause();
			}
		});
		
		Button stopButton = (Button) mediaPlayerView.findViewById(R.id.stop_button);
		stopButton.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				stop();
			}
		});
		
		Button volUpButton = (Button) mediaPlayerView.findViewById(R.id.volup_button);
		volUpButton.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				volUp();
			}
		});
		
		Button volDownButton = (Button) mediaPlayerView.findViewById(R.id.voldown_button);
		volDownButton.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				volDown();
			}
		});
		
	}
	
	private void updateSpinner(){
		Spinner tracksSpinner = (Spinner) mediaPlayerView.findViewById(R.id.spinner1);
		List<String> list = new ArrayList<String>();
		
		if(intentUrl != null)
			list.add(intentUrl);
		else
			list.add("No tracks");
		
		ArrayAdapter<String> dataAdapter = new ArrayAdapter<String>(ctx, android.R.layout.simple_spinner_item, list);
		dataAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
		tracksSpinner.setAdapter(dataAdapter);
	}
	
	public void setUrl(String url){
		intentUrl = url;
		
		if(intentUrl != null)
			play(intentUrl);
		
		updateSpinner();
	}
	
	public void sendMessage(String message){
		MessageTask t = new MessageTask();
		t.execute(message);
	}
	
	public void play(String url){
		sendMessage("PLAY " + url);
	}
	
	public void pause(){
		sendMessage("PAUSE");
	}
	
	public void stop(){
		sendMessage("CLOSE");
	}
	
	public void volUp(){
		sendMessage("VOL-UP");
	}
	
	public void volDown(){
		sendMessage("VOL-DOWN");
	}
	
	public void seekUp(){
		sendMessage("SEEK-UP");
	}
	
	public void seekDown(){
		sendMessage("SEEK-DOWN");
	}
	
	public void seekFW(){
		sendMessage("SEEK-FW");
	}
	
	public void seekBW(){
		sendMessage("VOL-BW");
	}
	
	
	
	
	private class MessageTask extends AsyncTask<String, Float, Boolean> {

		@Override
		protected Boolean doInBackground(String... params) {
			
			String message = (String) params[0]; 
			Socket s 			= null;
			
			/*
			BufferedInputStream  reader = null;
			BufferedOutputStream writer = null;
			*/
			
			InputStream  reader = null;
			OutputStream writer = null;
			
			try {
				s = new Socket(BBHOST, PORT);
				
				reader = new BufferedInputStream(s.getInputStream());
				writer = new BufferedOutputStream(s.getOutputStream());
				
				/*
				// Se lee e ignora la cabecera del protocolo
				while(reader.available() > 0) reader.read();
				
				// Se envia el mensaje
				writer.write(message.getBytes());
				writer.flush();
				
				// Se lee e ignora el mensaje de respuesta
				while(reader.available() > 0) reader.read();
				*/
				
				reader = s.getInputStream();
				writer = s.getOutputStream();
				
				int size = s.getReceiveBufferSize();
				byte [] recDataBuff = new byte [size];
				reader.read(recDataBuff, size, 0);
				// Log.d("DATA",recDataBuff.toString());
				
				// Se envia el mensaje
				writer.write(message.getBytes());
				writer.flush();
				
				// Se lee e ignora el mensaje de respuesta
				size = s.getReceiveBufferSize();
				recDataBuff = new byte [size];
				reader.read(recDataBuff, size, 0);
				String decoded = new String(recDataBuff);
				
				Log.d("DATA",decoded);
				
			} catch (Exception e) {
				// TODO Auto-generated catch block
				Log.e("MediaPlayer",Log.getStackTraceString(e));
			}
			finally
			{
				if (reader != null){
					try{
						reader.close();
					}catch(Exception e){ /* Nothing to do */}
				}
				
				if (s != null){
					try{
						s.close();
					}catch(Exception e){ /* Nothing to do */}
				}
			}
			
			return true;
		}
	}
}
