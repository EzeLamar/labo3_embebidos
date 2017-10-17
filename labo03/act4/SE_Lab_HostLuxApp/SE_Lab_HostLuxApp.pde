
/**
 * Ejemplo de skecth Processing para el desarrollo del Laboratorio.
 * Sistemas Embebidos - 1º Cuatrimestre de 2016
 *
 * Este skecth implementa un programa en el host que tiene la capacidad de
 * graficar funciones que evolucionan en el tiempo y además permite la
 * presentación de datos simples mediante Labels, y la captura de eventos
 * del usuario mediante la implementación de botones simples.
 *
 * La aplicación divide la ventana en 2 regiones, una de dibujado y otra 
 * donde se ubican los botones y etiquetas de información.
 *
 * Autor: Sebastián Escarza
 * Sistemas Embebidos - 1ºC de 2016.
 */


// Declaraciones para graficar funciones...
int cosVal;
int cantValues;
ScrollingFcnPlot f1,f2;

//Botones...
RectButton rectBtn1, rectBtn2;
PFont myFont;

//Etiquetas textuales...
boolean alert = false;
boolean prealert = false;
Label lbl1, lbl2, lbl3, lbl4, lbl5, lbl6, lbl7, lbl8, lbl9, lbl10, lbl11;
Label yVal1,yVal2,yVal3;

//Ventana y viewports...
int pgFunctionViewportWidth = 480;
int pgControlViewportWidth = 120;
int pgViewportsHeight = 240;
int valorActualLux=0;

void settings () {

    size(pgFunctionViewportWidth+pgControlViewportWidth, pgViewportsHeight);
}

void setup() {

  //Se define el tamaño de la ventana de la aplicación... 
  //size(pgFunctionViewportWidth+pgControlViewportWidth, pgViewportsHeight);
  
  //Se inicializan los arreglos para almacenar las funciones...
  cantValues = 200;//pgFunctionViewportWidth;
  f1 = new ScrollingFcnPlot(cantValues,color(100,0,0),-1,1);
  f2 = new ScrollingFcnPlot(cantValues,color(0,0,100),0,height);
  
  //Se inicializan los botones...
  //rectBtn1 = new RectButton(pgFunctionViewportWidth+10,10,60,40,color(102),color(50),color(255),"B1");
  rectBtn2 = new RectButton(pgFunctionViewportWidth+10,60,60,40,color(102),color(50),color(255),"A2");
  
  //Se inicializan los labels...
  lbl1 = new Label(pgFunctionViewportWidth+10,110,color(255),"Status: ");
  lbl2 = new Label(pgFunctionViewportWidth+60,110,color(255),"-");
  lbl3 = new Label(pgFunctionViewportWidth+10,130,color(255),"L.Actual: ");
  lbl4 = new Label(pgFunctionViewportWidth+70,130,color(255),"10l");
  lbl5 = new Label(pgFunctionViewportWidth+10,150,color(255),"L.Max: ");
  lbl6 = new Label(pgFunctionViewportWidth+70,150,color(255),"10l");
  lbl7 = new Label(pgFunctionViewportWidth+10,170,color(255),"L.Min: ");
  lbl8 = new Label(pgFunctionViewportWidth+70,170,color(255),"10l");
  lbl9 = new Label(pgFunctionViewportWidth+10,190,color(255),"L.Prom: ");
  lbl10 = new Label(pgFunctionViewportWidth+70,190,color(255),"10l");
  lbl11 = new Label(pgFunctionViewportWidth+10,210,color(255,0,0),"ALERTA!!!");
  
  yVal1 = new Label(10,5,color(255),"+100");
  yVal2 = new Label(10,(pgViewportsHeight-20)/2,color(255),"0");
  yVal3 = new Label(10,pgViewportsHeight-25,color(255),"-100");
  
  //Inicializa el font de la GUI...
  myFont = createFont("FFScala", 14);
  textFont(myFont);
  
}

void draw() {
  //obtengo los valores de LUX a partir de una consulta al GALILEO:
  //consulta OBTENER_TODO:
  //-----
  //muestro los valores en los labels y el valor actual de lux se lo asigna al grafico...
  
  
  //Se actualizan las funciones de ejemplo (f1: coseno(x), f2: función que depende de la posición Y del mouse)
  float amount = map(cosVal, 0, cantValues, 0, 2*PI);
  f1.updateValue(cos(amount));
  f2.updateValue(valorActualLux);
  
  //Se incrementa el ángulo de la función coseno...
  cosVal = (cosVal + 1) % cantValues;
  
  //Permite expermientar con la velocidad de scroll al actualizar más lentamente los valores...
  //delay(50);
     
  //Rendering de la interface...
  background(125);
  stroke(0);
  noFill();
 
  //Dibuja las funciones...
  f1.displayIntoRect(30,10,pgFunctionViewportWidth-10,pgViewportsHeight-10);
  f2.displayIntoRect(30,10,pgFunctionViewportWidth-10,pgViewportsHeight-10);
  
  //Procesa eventos de MouseOver...
  //rectBtn1.update();
  rectBtn2.update();
  
  //Procesa las entradas (botones)
  if(mousePressed) {
    /*if(rectBtn1.pressed()) {
      rectBtn1.currentcolor = color(0,100,0);
      lbl2.caption = "Btn1";
      prealert = true;
    } 
    else*/
    if(rectBtn2.pressed()) {
      rectBtn2.currentcolor = color(0,100,0);
      lbl2.caption = "Btn2";
      prealert = true;
    }
  }
  
  
  //Dibuja el eje X y el recuadro de los gráficos...
  stroke(0);
  line(30, pgViewportsHeight/2, pgFunctionViewportWidth-10, pgViewportsHeight/2);
  rect(30,10,pgFunctionViewportWidth-40,pgViewportsHeight-20);
  
  //Se dibujan los botones...
  //rectBtn1.display();
  rectBtn2.display();
  
  //Se dibuja texto adicional...(labels, etc)
  lbl1.display();
  lbl2.display();
  lbl3.display();
  lbl4.display();
  lbl5.display();
  lbl6.display();
  lbl7.display();
  lbl8.display();
  lbl9.display();
  lbl10.display();

  if (alert) lbl11.display();
  
  yVal1.display();
  yVal2.display();
  yVal3.display();  
}

void mouseReleased()
{
  //Si se pulsó algún botón y se completa el click, se hace el toggle sobre la etiqueta de alerta...
  println("se apreto el A2");
  if(prealert)
  {
    alert=!alert;
    prealert = false;
  }
}

public void keyPressed(){
    println("Boton apretado: " + keyCode);
    if (key == 'w')
    {
        println("botonUp()");
        valorActualLux+=5;
    }
    else if (key == 's')
    {
        println("botonDown()");
        valorActualLux-=5;
    }
    else if (key == 'a')
    {
        println("botonLeft()");
    }
    else if (key == 'd')
    {
        println("botonRight()");
    }
}