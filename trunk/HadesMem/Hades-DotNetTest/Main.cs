﻿using System;
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
        private string[] MainArgs;

        public Main(string[] args)
        {
            MainArgs = args;
            InitializeComponent();
        }

        private void Main_Load(object sender, EventArgs e)
        {
            uint SessionId = HadesVM.GetSessionId();
            LstOutput.Items.Add("Session ID: " + SessionId.ToString());
            foreach (string arg in MainArgs)
                LstOutput.Items.Add(arg);
            HadesVM.AddFrameHandler(OnFrame);
        }

        public void OnFrame()
        {
            LstOutput.Items.Add("OnFrame!");
            //MessageBox.Show("OnFrame!");
        }
    }
}
