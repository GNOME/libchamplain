 using System;
 using Gtk;
 using Champlain;
 
 public class ChamplainHelloWorld {
  
   public static void Main() {
     Clutter.ClutterRun.Init();
     Application.Init();
 
     Window mainWin = new Window("Hello World!");
     mainWin.Resize(200,200);
  
     Champlain.View view = new Champlain.View();
     Widget mapWidget = new ViewEmbed(view);
   
     mainWin.Add(mapWidget);
     mainWin.ShowAll();
     
     Application.Run();   
   }
 }
