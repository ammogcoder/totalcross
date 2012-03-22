package tc.samples.service.im.controller;

import totalcross.io.*;
import totalcross.net.*;
import totalcross.sys.*;
import totalcross.ui.*;
import totalcross.ui.event.*;

public class MailService extends MainWindow implements Runnable
{
   public void run()
   {
      String outFolder = Settings.onJavaSE ? "/msg" : Settings.platform.equals(Settings.ANDROID) ? "/sdcard/msg" : "/msg";
      try 
      {
         if (!new File(outFolder).exists())
            new File(outFolder).createDir();
      } catch (Exception e) {e.printStackTrace();}
      while (true)
      {
         byte []buf = new byte[1024];
         try
         {
            for (int i = 1; i < 1000; i++)
            {
               String fileName = "toclient"+i+".txt";
               Vm.debug("Looking at server for file "+fileName);
               HttpStream.Options options = new HttpStream.Options();
               options.openTimeOut = 30000;
               options.readTimeOut = options.writeTimeOut = 20000;
               HttpStream hs = new HttpStream(new URI("http://www.totalcross.com/msg/"+fileName),options);
               if (hs.responseCode == 404) // page not found
               {
                  hs.close();
                  break;
               }
               int len = hs.contentLength;
               Vm.debug("Receiving with len "+len);
               if (buf.length < len)
                  buf = new byte[len];
               if (readFully(hs,buf,len))
               {
                  File out = new File(Convert.appendPath(outFolder,fileName),File.CREATE_EMPTY);
                  out.writeBytes(buf,0,len);
                  out.close();
               }
               Vm.debug("File written");
               hs.close();
            }
         }
         catch (Exception e)
         {
            e.printStackTrace();
         }
         Vm.debug("waiting 120 seconds");
         Vm.sleep(2*60*1000); // wait 120 seconds
      }
   }
   
   private boolean readFully(Stream hs, byte[] buf, int len) throws IOException
   {
      for (int ofs = 0, r=0; ofs < len; ofs += r)
      {
         r = hs.readBytes(buf,ofs,len-ofs);
         if (r < 0)
            return false;
      }
      return true;
   }
   
   TimerEvent te;
   
   public void initUI()
   {
      te = addTimer(15*1000);
      add(new Button("running"),CENTER,CENTER);
   }
   
   boolean minimized;
   public void onEvent(Event e)
   {
      if (e.type == TimerEvent.TRIGGERED && te.triggered)
      {
         if (minimized)
            minimize();
         else
            restore();
         minimized = !minimized;
      }
   }
}
