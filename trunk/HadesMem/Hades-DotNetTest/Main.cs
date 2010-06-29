using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using HadesAD;

namespace Hades_DotNetTest
{
    public partial class Main : Form
    {
        public Main()
        {
            InitializeComponent();
        }

        private void Main_Load(object sender, EventArgs e)
        {
            HadesVM.AddFrameHandler(OnFrame);
        }

        public void OnFrame()
        {
            LstOutput.Items.Add("OnFrame!");
            //MessageBox.Show("OnFrame!");
        }
    }
}
