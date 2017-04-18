using System;
using System.IO;

namespace Mandelbrot 
{
	public class Mandelbrot
	{
		const int BAILOUT = 16;
		const int MAX_ITERATIONS = 100000;
		Mandelbrot()
		{
			DateTime d1 = DateTime.Now;
			for (int y = -39; y < 39; y++) {
				for (int x = -39; x < 39; x++) {

					if (iterate(x/40.0, y/40.0) == 0) 
						Console.Write("*");
					else
						Console.Write(" ");

				}
				Console.WriteLine();
			}
			DateTime d2 = DateTime.Now;
			long diff = d2.Ticks - d1.Ticks;
			Console.WriteLine("\nC# Elapsed {0:N2} s", diff / 1e7);
		}

		int iterate(double x, double y)
		{
			double cr = y - 0.5;
			double ci = x;
			double zr = 0.0, zi = 0.0;
			int i = 0;
			while (true) {
				i++;
				var temp = zr * zi;
				var zr2 = zr * zr;
				var zi2 = zi * zi;
				zr = zr2 - zi2 + cr;
				zi = temp + temp + ci;
				if (zi2 + zr2 > BAILOUT)
					return i;
				if (i > MAX_ITERATIONS)
					return 0;
			}
		}
		
		public static void Main()
		{
			TextWriter tmp = Console.Out;
			var sw = new StringWriter();
			Console.SetOut(sw);
			var m = new Mandelbrot();
			Console.SetOut(tmp);
			sw.Close();	
			Console.WriteLine(sw.ToString());
		}
	}
}


