﻿using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using System.Runtime.InteropServices;
using System.IO;
using System.Diagnostics;
using GOClrDll;
namespace GOGPY
{
    public partial class FormMain : Form
    {
        #region 公共变量
        private Capture cam;
        string strPath = "";
        int iparam = 32;
        IntPtr m_ip = IntPtr.Zero;
        Image srcImage;
        GOCsharpHelper gocsharphelper = new GOCsharpHelper();
        #endregion

      
        public FormMain()
        {
            InitializeComponent();
            const int VIDEODEVICE = 0; // zero based index of video capture device to use
            const int VIDEOWIDTH = 640; // Depends on video device caps
            const int VIDEOHEIGHT = 480; // Depends on video device caps
            const int VIDEOBITSPERPIXEL = 24; // BitsPerPixel values determined by device

            cam = new Capture(VIDEODEVICE, VIDEOWIDTH, VIDEOHEIGHT, VIDEOBITSPERPIXEL, picPreview);
        }
        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (components != null)
                {
                    components.Dispose();
                }
            }
            base.Dispose(disposing);

            if (m_ip != IntPtr.Zero)
            {
                Marshal.FreeCoTaskMem(m_ip);
                m_ip = IntPtr.Zero;
            }
        }

        #region 图像处理wraper
        class GOCsharpHelper
        {
            GOClrClass client = new GOClrClass();
            string strResult1 = null;
            string strResult2 = null;
            string strResult0 = null;
            //输入参数是string或bitmap
            public Bitmap ImageProcess0(string ImagePath, int iparam)
            {
                Image ImageTemp = Bitmap.FromFile(ImagePath);
                return ImageProcess0(ImageTemp, iparam);
            }
            //输出结果是bitmap
            public Bitmap ImageProcess0(Image image, int iparam)
            {
                MemoryStream ms = new MemoryStream();
                image.Save(ms, System.Drawing.Imaging.ImageFormat.Jpeg);
                byte[] bytes = ms.GetBuffer();
                strResult0 = client.ConvetAndSplit0(bytes, iparam);
                Image ImageResult = Bitmap.FromFile(strResult0);
                return (Bitmap)ImageResult;
            }

            //输入参数是string或bitmap
            public Bitmap ImageProcess1(string ImagePath, int iparam)
            {
                Image ImageTemp = Bitmap.FromFile(ImagePath);
                return ImageProcess1(ImageTemp, iparam);
            }
            //输出结果是bitmap
            public Bitmap ImageProcess1(Image image, int iparam)
            {
                MemoryStream ms = new MemoryStream();
                image.Save(ms, System.Drawing.Imaging.ImageFormat.Jpeg);
                byte[] bytes = ms.GetBuffer();
                strResult1 = client.ConvetAndSplit1(bytes, iparam);
                Image ImageResult = Bitmap.FromFile(strResult1);
                return (Bitmap)ImageResult;
            }

            //输入参数是string或bitmap
            public Bitmap ImageProcess2(string ImagePath, int iparam)
            {
                Image ImageTemp = Bitmap.FromFile(ImagePath);
                return ImageProcess2(ImageTemp, iparam);
            }
            //输出结果是bitmap
            public Bitmap ImageProcess2(Image image, int iparam)
            {
                MemoryStream ms = new MemoryStream();
                image.Save(ms, System.Drawing.Imaging.ImageFormat.Jpeg);
                byte[] bytes = ms.GetBuffer();
                strResult2 = client.ConvetAndSplit2(bytes, iparam);
                Image ImageResult = Bitmap.FromFile(strResult2);
                return (Bitmap)ImageResult;
            }

            public void Clear()
            {
                if (File.Exists(strResult1))
                    File.Delete(strResult1);
                if (File.Exists(strResult2))
                    File.Delete(strResult2);
            }
        }
        #endregion

        private void btnCapture_Click(object sender, EventArgs e)
        {
            Cursor.Current = Cursors.WaitCursor;

            // Release any previous buffer
            if (m_ip != IntPtr.Zero)
            {
                Marshal.FreeCoTaskMem(m_ip);
                m_ip = IntPtr.Zero;
            }

            // capture image
            m_ip = cam.Click();//click就是采集吗？
            Bitmap b = new Bitmap(cam.Width, cam.Height, cam.Stride, PixelFormat.Format24bppRgb, m_ip);

            // If the image is upsidedown
            b.RotateFlip(RotateFlipType.RotateNoneFlipY);
            pictureBox1.Image = b;
            //处理测试 3 8 7
            srcImage = b;
            if (pictureBox3.Image != null)
                pictureBox3.Image.Dispose();
            if (pictureBox8.Image != null)
                pictureBox8.Image.Dispose();
            if (pictureBox7.Image != null)
                pictureBox7.Image.Dispose();

            Image image0 = gocsharphelper.ImageProcess0(srcImage, iparam);
            pictureBox3.Image = image0;
            Image image1 = gocsharphelper.ImageProcess1(srcImage, iparam);
            pictureBox8.Image = image1;
            Image image2 = gocsharphelper.ImageProcess2(srcImage, iparam);
            pictureBox7.Image = image2;

            Cursor.Current = Cursors.Default;
        }

        private void FormMain_FormClosing(object sender, FormClosingEventArgs e)
        {
            cam.Dispose();

            if (m_ip != IntPtr.Zero)
            {
                Marshal.FreeCoTaskMem(m_ip);
                m_ip = IntPtr.Zero;
            }

            //ReloadAllUnits();
            gocsharphelper.Clear();
        }
    }
}
