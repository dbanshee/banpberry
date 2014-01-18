package banshee.banpberrycontroller;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.MalformedURLException;
import java.net.Socket;
import java.net.URL;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Stack;
import java.util.TreeMap;

import net.htmlparser.jericho.Element;
import net.htmlparser.jericho.HTMLElementName;
import net.htmlparser.jericho.Source;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.AsyncTask;
import android.util.Log;
import android.view.View;
import android.view.View.OnLongClickListener;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

public class MediaBrowser {
	
//	private static final String BANPBERRY_IP = "192.168.15.1";
//	private static final String BANPTOWER_IP = "192.168.15.6";
	
	// Agregar en fichero de configuracion
	private static final String BANPBERRY_IP = "192.168.15.1";
	private static final String BANPTOWER_IP = "192.168.1.39";

	private Context ctx 			= null;
	private View mediaBrowserView 	= null;
	
	Map<String, String> rootElems 	= new TreeMap<String, String>();
	
	Stack<String> locationStack 	= new Stack<String>();
	List<String> listElems			= null;
	List<String> listRefs			= null;
	
	
	public MediaBrowser(Context ctx){
		this.ctx = ctx;
		
		rootElems.put("Banpberry HTTP", "http://"+BANPBERRY_IP+"/bbsite/");
		rootElems.put("BanTower  HTTP", "http://"+BANPTOWER_IP+"/banhttp/");
	}
	
	public void setView(View mediaBrowserView){
		this.mediaBrowserView 	= mediaBrowserView;
		
		String locationText;
		
		if(/*listElems == null && */locationStack.isEmpty()){
			listElems 	= new ArrayList<String>(rootElems.keySet());
			//listRefs  	= listElems;
			locationText = "";
		}else{
			locationText = locationStack.lastElement();
		}
		
		TextView locationView = (TextView) mediaBrowserView.findViewById(R.id.locationText);
		locationView.setText(locationText);
		
		ListView mediaListView = (ListView) mediaBrowserView.findViewById(R.id.mediaItemList);
		mediaListView.setAdapter(new ArrayAdapter<String>(ctx, R.layout.media_browser_item_view, listElems));
		mediaListView.setOnItemClickListener(new MediaBrowserItemListener());
	}
	
	void selectItem(int pos, boolean longCLick){
		String action = "INTO"; // Pasar a enum
		
		String newLocation = null;
		
		if(locationStack.isEmpty()){
			// Recupero la ruta inicial
			newLocation = new ArrayList<String>(rootElems.values()).get(pos);
			locationStack.push(newLocation);
		}else{
			// Recupero el nombre y genero la nueva ruta
			String elemName = listRefs.get(pos);
			
			if(elemName.equalsIgnoreCase("..")){
				locationStack.pop();
				
				if(locationStack.isEmpty())
					listElems = new ArrayList<String>(rootElems.keySet());
				else{
					action = "BACK";
					newLocation = locationStack.lastElement();
				}
			}else if(!elemName.endsWith("/")){
				action = "FILE";
				newLocation = locationStack.lastElement() + elemName;
			}else{
				action = "INTO";
				newLocation = locationStack.lastElement() + elemName;
				locationStack.push(newLocation);
			}
		}
		
		
		if(action.equalsIgnoreCase("FILE")){
			// Reproducir media
			
			if(!longCLick){
				((BBCMain) ctx).getMediaPlayer().setUrl(newLocation);
				Toast.makeText(ctx, "Playing : "+newLocation, Toast.LENGTH_LONG).show();
			}else{
				// Intent para prebisualizar media
				Intent intent = new Intent();
				intent.setAction(Intent.ACTION_VIEW);
				intent.setData(Uri.parse(newLocation));
				ctx.startActivity(intent);
			}
			
			
		}else if(action.equalsIgnoreCase("BACK") || action.equalsIgnoreCase("INTO")){
			try{
				if(newLocation != null){
					
					List<String> [] locationData = retrieveLocation(newLocation);
					listRefs  = locationData[0]; // Se almacenan las referencias
					listRefs.add(0, "..");
					listElems = locationData[1]; // Se muestran los nombres
					listElems.add(0, "..");
				}
				
				TextView locationView = (TextView) mediaBrowserView.findViewById(R.id.locationText);
				locationView.setText(newLocation);
				
				ListView mediaListView = (ListView) mediaBrowserView.findViewById(R.id.mediaItemList);
				mediaListView.setAdapter(new ArrayAdapter<String>(ctx, R.layout.media_browser_item_view, listElems));
				
				MediaBrowserItemListener mediaBList = new MediaBrowserItemListener();
				mediaListView.setOnItemClickListener(mediaBList);
				mediaListView.setOnItemLongClickListener(mediaBList);
			}catch(Exception e){
				Log.e("MediaBrowser",Log.getStackTraceString(e));
			}
		}
	} 
	
	public List<String> [] retrieveLocation(String url) throws Exception {
		
		LocationTask t = new LocationTask();
		t.execute(url);
		List<String> [] res = t.get(); 
		
		
		if(res == null)
			throw new Exception("Error restrieving location : " + url);
		
        return res;
	}
	
	private class MediaBrowserItemListener implements ListView.OnItemClickListener, ListView.OnItemLongClickListener {
        @Override
        public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
            selectItem(position, false);
        }

		@Override
		public boolean onItemLongClick(AdapterView<?> arg0, View arg1, int position, long arg3) {
			selectItem(position, true);
			return true;
		}
	}
	
	
	private class LocationTask extends AsyncTask<String, Float, List<String> []>  {

		@Override
		protected List<String> [] doInBackground(String... params) {
			
			String url = (String) params[0]; 
		
			List<String> [] res = new List [2];
			
			res[0] = new ArrayList<String>(); 		// Lista de referencias
			res[1] = new ArrayList<String>();		// Lista de nombres
			
			try{
				Source source = new Source(new URL(url));

		        source.fullSequentialParse();
		        
		        List<Element> linkElements = source.getAllElements(HTMLElementName.A); 
		        for (Element linkElement : linkElements) { 
			        String href = linkElement.getAttributeValue("href"); 
			        if (href != null && !linkElement.getContent().toString().equalsIgnoreCase("Parent Directory") && 
			        					 linkElement.getParentElement().getStartTag().toString().equalsIgnoreCase("<td>")) { 
		                res[0].add(href);
		                res[1].add(linkElement.getContent().toString());
			        } 
		        }
		        return res;
	        } catch (MalformedURLException e) {
				Log.e("MediaBrowser",Log.getStackTraceString(e));
				res = null;
			} catch (IOException e) {
				Log.e("MediaBrowser",Log.getStackTraceString(e));
				res = null;
			} 
			
			return res;
			
		}
	}
	
}


