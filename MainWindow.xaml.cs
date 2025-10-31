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

namespace lord13;

public partial class MainWindow : Window
{
    bool isDraw = false;
    private void Error()
    {
        MessageBox.Show("ты дурачина");
    }

    private void Brush(object sender, RoutedEventArgs e)
    {

    }

    private void Eraser(object sender, RoutedEventArgs e)
    {

    }

    private void Draw_down(object sender, MouseButtonEventArgs e)
    {
        isDraw = true;
    }

    private void Draw_move(object sender, RoutedEventArgs e)
    {
        if (isDraw == true)
        {
            isDraw = false;
        }
    }
    
    private void Draw_up(object sender, MouseButtonEventArgs e)
    {
        isDraw = false;
    }
}