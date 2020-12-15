using System;
using System.Drawing;
using System.Windows.Forms;

namespace lab7_winforms_
{
    public partial class MessageElement : UserControl
    {
        public MessageElement()
        {
            InitializeComponent();

            MouseEnter += MessageElement_MouseEnter;
            foreach (Control c in Controls)
                c.MouseEnter += new EventHandler(MessageElement_MouseEnter);

            MouseLeave += MessageElement_MouseLeave;
            foreach (Control c in Controls)
                c.MouseLeave += new EventHandler(MessageElement_MouseLeave);
        }

        public MessageElement(string messageSubject, string messageFrom, DateTime messageDateTime) : this()
        {
            messageSubjectLabel.Text = messageSubject;
            messageFromLabel.Text = messageFrom;
            dateTimeLabel.Text = messageDateTime.ToString("MM/dd/yy\nHH:mm");
        }

        private void MessageElement_MouseEnter(object sender, EventArgs e)
        {
            BackColor = SystemColors.ControlDark;
        }

        private void MessageElement_MouseLeave(object sender, EventArgs e)
        {
            BackColor = SystemColors.Control;
        }
    }
}
