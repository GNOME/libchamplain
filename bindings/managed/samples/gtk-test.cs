 using System;
 using Gtk;
 using Champlain;
 
 public class ChamplainHelloWorld {
  
   public static void Main() {
     Application.Init();
     Clutter.ClutterRun.Init();
 
     Window mainWin = new Window("Hello World!");
     mainWin.Resize(200,200);
  
     Champlain.View view = new Champlain.View();
     Widget mapWidget = new ViewEmbed(view);
   
     mainWin.Add(mapWidget);
     mainWin.ShowAll();
     
     Application.Run();
   }
 }
