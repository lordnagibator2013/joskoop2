using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.IO;
using Microsoft.Win32;
using System.IO.Enumeration;

namespace lord13;

public partial class MainWindow : Window
{
    bool isDraw = false;
    short thickness = 3;
    Color currentColor = Color.FromRgb(0, 0, 0);
    Point point1;
    private bool[] tools = { false, false, false, false, false }; //brush 0, eraser 1, line 2, ellipse 3, rect 4
    private void Truthify(bool[] tools, byte j)
    {
        for (int i = 0; i < tools.Length; i++)
        {
            tools[i] = false;
        }
        tools[j] = true;
    }

    private bool NoneTool(bool[] tools)
    {
        foreach (bool e in tools)
        {
            if (e == true) { return true; }
        }
        return false;
    }
    
    private void Error()
    {
        MessageBox.Show("ты дурачина");
    }

    private void NewThick(object sender, RoutedEventArgs e)
    {
        string strthick = Thick.Text;
        if (uint.TryParse(strthick, out uint size))
        {
            if (size > 60) { thickness = 60; }
            else { thickness = Convert.ToInt16(size); }
        }
        else{ Error(); }
    }

    private void Brush(object sender, RoutedEventArgs e)
    {
        Truthify(tools, 0);
        UpdateBC();
    }
    private void Eraser(object sender, RoutedEventArgs e)
    {
        Truthify(tools, 1);
        UpdateBC();
    }

    private void Line(object sender, RoutedEventArgs e)
    {
        Truthify(tools, 2);
        UpdateBC();
    }

    private void Ellipse(object sender, RoutedEventArgs e)
    {
        Truthify(tools, 3);
        UpdateBC();
    }

    private void Rect(object sender, RoutedEventArgs e)
    {
        Truthify(tools, 4);
        UpdateBC();
    }

    private void Clear(object sender, RoutedEventArgs e)
    {
        Canvass.Children.Clear();
        Truthify(tools, 0);
    }

    private void Draw(Point x1, Point x2, Color color)
    {
        SolidColorBrush brush = new SolidColorBrush(color);
        Line line = new Line
        {
            X1 = x1.X,
            Y1 = x1.Y,
            X2 = x2.X,
            Y2 = x2.Y,
            Stroke = brush,
            StrokeThickness = thickness,
            StrokeStartLineCap = PenLineCap.Round,
            StrokeEndLineCap = PenLineCap.Round,
            StrokeLineJoin = PenLineJoin.Round
        };
        Canvass.Children.Add(line);
    }

    private void DrawRect(Point x1, Point x2, Color color)
    {
        Point pointx1y2 = new Point(x1.X, x2.Y);
        Point pointx2y1 = new Point(x2.X, x1.Y);
        Draw(x1, pointx1y2, color);
        Draw(x1, pointx2y1, color);
        Draw(pointx1y2, x2, color);
        Draw(pointx2y1, x2, color);
    }
    
    private void DrawEllipse(Point p1, Point p2, Color color)
    {
        SolidColorBrush brush = new SolidColorBrush(color);
        Ellipse ellipse = new Ellipse
        {
            Stroke = brush,
            StrokeThickness = thickness,
            Fill = Brushes.Transparent
        };
        
        double x = Math.Min(p1.X, p2.X);
        double y = Math.Min(p1.Y, p2.Y);
        double width = Math.Abs(p2.X - p1.X);
        double height = Math.Abs(p2.Y - p1.Y);
        
        Canvas.SetLeft(ellipse, x);
        Canvas.SetTop(ellipse, y);
        ellipse.Width = width;
        ellipse.Height = height;
        
        Canvass.Children.Add(ellipse);
    }
    
    private void ChangeColor(object sender, RoutedEventArgs e)
    {
        Button button = (Button)sender;
        currentColor = ((SolidColorBrush)button.Background).Color;
    }

    private void Draw_down(object sender, MouseButtonEventArgs e)
    {
        if (!NoneTool(tools)){ return; }
        isDraw = true;
        point1 = e.GetPosition(Canvass);
    }

    private void Draw_move(object sender, MouseEventArgs e)
    {
        if (!isDraw) { return; }
        if (tools[3] || tools[4] || tools[2]) { return; }
        Point point2 = e.GetPosition(Canvass);
        if (tools[0]) { Draw(point1, point2, currentColor); }
        if (tools[1]) { Draw(point1, point2, Color.FromArgb(255, 255, 255, 255)); }
        point1 = point2;
    }

    private void Draw_up(object sender, MouseButtonEventArgs e)
    {
        isDraw = false;
        if (tools[2] || tools[3] || tools[4])
        {
            Point point2 = e.GetPosition(Canvass);
            if (tools[2])
            {
                Draw(point1, point2, currentColor);
            }
            if (tools[3])
            {
                DrawEllipse(point1, point2, currentColor);
            }
            if (tools[4])
            {
                DrawRect(point1, point2, currentColor);
            }
        }
    }

    public MainWindow()
    {
        InitializeComponent();
    }

    private void UpdateBC()
    {
        if (tools[0])
        {
            BrushB.Background = Brushes.LightBlue;
            EraserB.Background = Brushes.LightGray;
        }

        if (tools[1])
        {
            BrushB.Background = Brushes.LightGray;
            EraserB.Background = Brushes.LightBlue;
        }
        if (!tools[0] && !tools[1])
        {
            BrushB.Background = Brushes.LightGray;
            EraserB.Background = Brushes.LightGray;
        }
    }

    private void SaveAsJpg(string filePath)
    {
        double width = Canvass.ActualWidth;
        double height = Canvass.ActualHeight;

        RenderTargetBitmap bitmap = new RenderTargetBitmap((int)width, (int)height, 96, 96, PixelFormats.Pbgra32);
        bitmap.Render(Canvass);

        JpegBitmapEncoder jpegEncoder = new JpegBitmapEncoder();
        jpegEncoder.QualityLevel = 100;
        jpegEncoder.Frames.Add(BitmapFrame.Create(bitmap));

        using (FileStream fileStream = new FileStream(filePath, FileMode.Create))
        {
            jpegEncoder.Save(fileStream);
        }
    }
    
    private void SavePic(object sender, RoutedEventArgs e)
    {
        if (string.IsNullOrEmpty(FileName.Text))
            {
                Error();
                return;
            }

        string desktopPath = Environment.GetFolderPath(Environment.SpecialFolder.Desktop);
        string fullPath = System.IO.Path.Combine(desktopPath, FileName.Text + ".jpg");

        SaveAsJpg(fullPath);
        MessageBox.Show("даблю");
    }
}