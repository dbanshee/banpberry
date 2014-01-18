package banshee.banpberrycontroller;

import java.util.Locale;

import android.os.Bundle;
import android.app.Activity;
import android.app.Fragment;
import android.app.SearchManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.support.v4.app.ActionBarDrawerToggle;
import android.support.v4.app.FragmentManager;
import android.support.v4.view.GravityCompat;
import android.support.v4.widget.DrawerLayout;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.Toast;

public class BBCMain extends Activity {

	private DrawerLayout 			mDrawerLayout;
    private ListView 				mDrawerList;
    private ActionBarDrawerToggle 	mDrawerToggle;

    private CharSequence 			mDrawerTitle;
    private CharSequence 			mTitle;
    private String [] 				mMenuOptionsTitles;
    
    private static MediaPlayer 		mPlayer;
    private static MediaBrowser		mBrowser;
    
    private static String 			intentUrl = null;
    
    public MediaPlayer getMediaPlayer(){
    	return mPlayer;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_bbcmain);
        
        mPlayer 	= new MediaPlayer(this);
        mBrowser 	= new MediaBrowser(this);
        
        
        MenuOptionFragment.setContext(this);

        mTitle = mDrawerTitle = getTitle();
        //mPlanetTitles = getResources().getStringArray(R.array.planets_array);
        mDrawerLayout = (DrawerLayout) findViewById(R.id.drawer_layout);
        mDrawerList = (ListView) findViewById(R.id.left_drawer);

        // set a custom shadow that overlays the main content when the drawer opens
        //mDrawerLayout.setDrawerShadow(R.drawable.ic_drawer, GravityCompat.START);
        
        // set up the drawer's list view with items and click listener
        mMenuOptionsTitles = getResources().getStringArray(R.array.menu_options_array);
        mDrawerList.setAdapter(new ArrayAdapter<String>(this, R.layout.drawer_list_item, mMenuOptionsTitles));
        mDrawerList.setOnItemClickListener(new DrawerItemClickListener());

        // enable ActionBar app icon to behave as action to toggle nav drawer
        getActionBar().setDisplayHomeAsUpEnabled(true);
        getActionBar().setHomeButtonEnabled(true);

        // ActionBarDrawerToggle ties together the the proper interactions
        // between the sliding drawer and the action bar app icon
        mDrawerToggle = new ActionBarDrawerToggle(
                this,                  	/* host Activity */
                mDrawerLayout,         	/* DrawerLayout object */
                R.drawable.ic_drawer,   /* nav drawer image to replace 'Up' caret */
                R.string.drawer_open,  	/* "open drawer" description for accessibility */
                R.string.drawer_close  	/* "close drawer" description for accessibility */
                ) {
        	
            public void onDrawerClosed(View view) {
                getActionBar().setTitle(mTitle);
                invalidateOptionsMenu(); // creates call to onPrepareOptionsMenu()
            }

            public void onDrawerOpened(View drawerView) {
                getActionBar().setTitle(mDrawerTitle);
                invalidateOptionsMenu(); // creates call to onPrepareOptionsMenu()
            }
        };
        mDrawerLayout.setDrawerListener(mDrawerToggle);

        if (savedInstanceState == null) {
            selectItem(0);
        }
        
        
//        //Creamos el adaptador
//        Spinner trackSpinner = (Spinner) findViewById(R.id.spinner1);
//        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this,R.array.menu_options_array,android.R.layout.simple_spinner_item);
//    	
//        //Añadimos el layout para el menú
//        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
//        
//        //Le indicamos al spinner el adaptador a usar
//        trackSpinner.setAdapter(adapter);
        
        
        // Se obtiene el intent de la llamada
        Intent intent = getIntent();

        // Si es compartido o abierto
        if(intent.getAction().equals(Intent.ACTION_SEND) || intent.getAction().equals(Intent.ACTION_VIEW)){
            //
            if(intent.getAction().equals(Intent.ACTION_SEND))
                intentUrl = intent.getStringExtra(Intent.EXTRA_TEXT);
            else
                intentUrl = intent.getDataString();
        }
        else if(intent.getAction().equals(Intent.ACTION_MAIN)){
        	// Nothing to do
        }
        
//        requestWindowFeature(Window.FEATURE_CUSTOM_TITLE);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.bbcmain, menu);
        return super.onCreateOptionsMenu(menu);
    }

    /* Called whenever we call invalidateOptionsMenu() */
    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        // If the nav drawer is open, hide action items related to the content view
        boolean drawerOpen = mDrawerLayout.isDrawerOpen(mDrawerList);
        menu.findItem(R.id.action_websearch).setVisible(!drawerOpen);
        return super.onPrepareOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
         // The action bar home/up action should open or close the drawer.
         // ActionBarDrawerToggle will take care of this.
    	
        if (mDrawerToggle.onOptionsItemSelected(item)) {
            return true;
        }
        
        // Handle action buttons
        switch(item.getItemId()) {
        	case R.id.action_websearch:
	            // create intent to perform web search for this planet
	            Intent intent = new Intent(Intent.ACTION_WEB_SEARCH);
	            intent.putExtra(SearchManager.QUERY, getActionBar().getTitle());
	            // catch event that there's no activity to handle intent
	            if (intent.resolveActivity(getPackageManager()) != null) {
	                startActivity(intent);
	            } else {
	                Toast.makeText(this, R.string.app_not_available, Toast.LENGTH_LONG).show();
	            }
	            return true;
	        default:
	            return super.onOptionsItemSelected(item);
        }
    }

    /* The click listener for ListView in the navigation drawer */
    private class DrawerItemClickListener implements ListView.OnItemClickListener {
        @Override
        public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
            selectItem(position);
        }
    }

    private void selectItem(int position) {
        // update the main content by replacing fragments
    	
        Fragment fragment = new MenuOptionFragment();
        Bundle args = new Bundle();
        args.putInt(MenuOptionFragment.ARG_OPTION_NUMBER, position);
        fragment.setArguments(args);

        android.app.FragmentManager fragmentManager = getFragmentManager();
        fragmentManager.beginTransaction().replace(R.id.content_frame, fragment).commit();

        // update selected item and title, then close the drawer
        mDrawerList.setItemChecked(position, true);
        setTitle(mMenuOptionsTitles[position]);
        mDrawerLayout.closeDrawer(mDrawerList);
    }

    @Override
    public void setTitle(CharSequence title) {
        mTitle = title;
        getActionBar().setTitle(mTitle);
    }

    /**
     * When using the ActionBarDrawerToggle, you must call it during
     * onPostCreate() and onConfigurationChanged()...
     */

    @Override
    protected void onPostCreate(Bundle savedInstanceState) {
        super.onPostCreate(savedInstanceState);
        // Sync the toggle state after onRestoreInstanceState has occurred.
        mDrawerToggle.syncState();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        // Pass any configuration change to the drawer toggls
        mDrawerToggle.onConfigurationChanged(newConfig);
    }

    /**
     * Fragment that appears in the "content_frame", shows a planet
     */
    public static class MenuOptionFragment extends Fragment {
        /*public*/ static final String ARG_OPTION_NUMBER = "menu_option_number";
        static Context ctx = null;
        
        public static void setContext(Context ctx){
        	MenuOptionFragment.ctx = ctx;
        }


        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
                Bundle savedInstanceState) {
        	
            int i = getArguments().getInt(ARG_OPTION_NUMBER);
            String option = getResources().getStringArray(R.array.menu_options_array)[i];
            
            View rootView;
            
            switch (i) {
            	case 0 : 
            		rootView = inflater.inflate(R.layout.media_player_view, container, false);
            		
        			mPlayer.setView(rootView);
            		mPlayer.setUrl(intentUrl);
            		intentUrl = null;
        			
            		break;
            	case 1 : 
            		rootView = inflater.inflate(R.layout.media_browser_view, container, false);
            		mBrowser.setView(rootView);
            		break;
            	case 2 : 
            		rootView = inflater.inflate(R.layout.lights_view, container, false);
            		break;
            	case 3 : 
            		rootView = inflater.inflate(R.layout.camera_view, container, false);
            		break;
            	case 4 : 
            		rootView = inflater.inflate(R.layout.banpotify_view, container, false);

            		// Temporal. Mover a Settings Class
            		
            		Button b = (Button) rootView.findViewById(R.id.button1);
            		b.setOnClickListener(new OnClickListener() {
            			@Override
            			public void onClick(View v) {
            	
                    		Intent res = new Intent();
                    		res.setComponent(new ComponentName("com.namelessdev.mpdroid","MPDApplication"));
                    		startActivity(res);
            			}
            		});
            		
            		break;
            	case 5 : 
            		rootView = inflater.inflate(R.layout.services_view, container, false);
            		break;
            	case 6 : 
            		rootView = inflater.inflate(R.layout.logs_view, container, false);
            		break;
            	case 7 : 
            		rootView = inflater.inflate(R.layout.settings_view, container, false);
            		break;
            	case 8 : 
            		rootView = inflater.inflate(R.layout.about_view, container, false);
            		break;
            	default :
            		rootView = inflater.inflate(R.layout.media_browser_view, container, false);
            		break;
            } 

//            int imageId = getResources().getIdentifier(planet.toLowerCase(Locale.getDefault()),
//                            "drawable", getActivity().getPackageName());
            
            
            getActivity().setTitle(option);
            return rootView;
        }
    }
}
