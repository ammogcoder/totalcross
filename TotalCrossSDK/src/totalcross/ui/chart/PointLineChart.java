/*********************************************************************************
 *  TotalCross Software Development Kit                                          *
 *  Copyright (C) 2000-2012 SuperWaba Ltda.                                      *
 *  All Rights Reserved                                                          *
 *                                                                               *
 *  This library and virtual machine is distributed in the hope that it will     *
 *  be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of    *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                         *
 *                                                                               *
 *  This file is covered by the GNU LESSER GENERAL PUBLIC LICENSE VERSION 3.0    *
 *  A copy of this license is located in file license.txt at the root of this    *
 *  SDK or can be downloaded here:                                               *
 *  http://www.gnu.org/licenses/lgpl-3.0.txt                                     *
 *                                                                               *
 *********************************************************************************/



package totalcross.ui.chart;

import totalcross.sys.*;
import totalcross.ui.*;
import totalcross.ui.event.ControlEvent;
import totalcross.ui.event.Event;
import totalcross.ui.event.KeyEvent;
import totalcross.ui.event.PenEvent;
import totalcross.ui.gfx.Color;
import totalcross.ui.gfx.Coord;
import totalcross.ui.gfx.Graphics;
import totalcross.ui.gfx.Rect;
import totalcross.util.Vector;

/** Abstract class used by points and line charts.
 * @see LineChart
 * @see XYChart 
 */
public abstract class PointLineChart extends Chart
{
   /** Contains all points that are currently painted on this chart */
   protected Vector points = new Vector();

   /** Flag to indicate whether the lines connecting points must be painted */
   public boolean showLines;

   /** Flag to indicate whether the points must be painted */
   public boolean showPoints;

   /** The current selected series */
   private int selectedSeries = -1;

   /** The current selected value from <code>selectedSeries</code> */
   private int selectedValue = -1;

   /** The radious of each point (in pixels) */
   public int pointR = 3;

   /** The line thickness. */
   public int lineThickness=1;

   /** Flag to indicate whether this chart is focused */
   private boolean hasFocus;

   public void onPaint(Graphics g)
   {
      // Draw lines
      if (showLines)
      {
         g.useAA = Settings.screenBPP > 8; 
         int thick = lineThickness;
         for (int i = series.size() - 1; i >= 0; i --) // for each series
         {
            g.foreColor = (((Series) series.items[i]).color);
            Vector v = (Vector) points.items[i]; // the series' points
            if (v != null)
            {
               for (int j = v.size() - 2; j >= 0; j --) // for each series point
               {
                  Coord c1 = (Coord) v.items[j];
                  Coord c2 = (Coord) v.items[j + 1];
                  for (int k = -(thick>>1); k <= (thick>>1); k++)
                     g.drawLine(c1.x, c1.y-k, c2.x, c2.y-k);
               }
            }
         }
         g.useAA = false;
      }

      // Draw points
      if (!showLines || showPoints || hasFocus)
      {
         for (int i = series.size() - 1; i >= 0; i --) // for each series
         {
            Series s = (Series) series.items[i]; // the series
            Vector v = (Vector) points.items[i]; // the series' points
            if (v != null)
            {
               for (int j = v.size() - 1; j >= 0; j--) // for each series point
               {
                  Coord c1 = (Coord) v.items[j];

                  int c = s.color;
                  if (selectedSeries == i && selectedValue == j)
                     c = Color.darker(c);

                  g.backColor = c;
                  g.fillCircle(c1.x, c1.y, pointR);
               }
            }
         }

         // Draw selection (text box)
         if (selectedSeries != -1)
         {
            Series s = (Series) series.items[selectedSeries];

            String text = Convert.toCurrencyString(s.yValues[selectedValue], yDecimalPlaces);
            if (s.xValues != null)
               text = "(" + Convert.toCurrencyString(s.xValues[selectedValue], xDecimalPlaces) + "," + text + ")";

            Coord c = (Coord) ((Vector) points.items[selectedSeries]).items[selectedValue];
            drawTextBox(g, c.x, c.y, text);
         }
      }
   }

   public void onEvent(Event e)
   {
      switch (e.type)
      {
         case PenEvent.PEN_DOWN:
         {
            PenEvent pe = (PenEvent) e;
            int xx = pe.x;
            int yy = pe.y;

            if (xx < (xAxisX1 - pointR) || xx > (xAxisX2 + pointR) || yy > (yAxisY1 + pointR) || yy < (yAxisY2 - pointR))
            {
               hasFocus = false;
               selectedSeries = -1;
               selectedValue = -1;
               Window.needsPaint = true;
            }
            else
            {
               hasFocus = true;
               selectedSeries = -1; // clear selection
               selectedValue = -1;

               Rect r = new Rect();
               r.width = r.height = pointR * 2;

               for (int i = series.size() - 1; i >= 0; i --) // for each series
               {
                  Vector v = (Vector) points.items[i]; // the series' points
                  if (v != null)
                  {
                     for (int j = v.size() - 1; j >= 0; j --) // for each series point
                     {
                        Coord c = (Coord) v.items[j];
                        r.x = c.x - pointR;
                        r.y = c.y - pointR;

                        if (r.contains(xx, yy))
                        {
                           selectedSeries = i;
                           selectedValue = j;

                           i = 0; // force outter for to exit
                           break;
                        }
                     }
                  }
               }

               Window.needsPaint = true;
            }
            break;
         }
         case KeyEvent.SPECIAL_KEY_PRESS:
         {
            KeyEvent ke = (KeyEvent) e;
            if (ke.isActionKey()) // release focus
            {
               isHighlighting = true;
               parent.requestFocus();
            }
            else if (ke.isNextKey()) // next point
            {
               if (selectedValue < ((Series) series.items[selectedSeries]).yValues.length - 1)
               {
                  selectedValue ++;
                  Window.needsPaint = true;
               }
               else if (selectedSeries < series.size() - 1)
               {
                  selectedSeries ++;
                  selectedValue = 0;
                  Window.needsPaint = true;
               }
            }
            else if (ke.isPrevKey()) // previous point
            {
               if (selectedValue > 0)
               {
                  selectedValue --;
                  Window.needsPaint = true;
               }
               else if (selectedSeries > 0)
               {
                  selectedSeries --;
                  selectedValue = ((Series) series.items[selectedSeries]).yValues.length - 1;
                  Window.needsPaint = true;
               }
            }
            break;
         }
         case ControlEvent.FOCUS_IN:
         {
            hasFocus = true;
            if (series.size() > 0 && ((Series) series.items[0]).yValues.length > 0)
            {
               selectedSeries = 0;
               selectedValue = 0;
               Window.needsPaint = true;
            }
            break;
         }
         case ControlEvent.FOCUS_OUT:
         {
            hasFocus = false;
            selectedSeries = -1;
            selectedValue = -1;
            Window.needsPaint = true;
            break;
         }
      }
   }
}