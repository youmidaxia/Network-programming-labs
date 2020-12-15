using AE.Net.Mail;
using System;
using System.Text;
using System.Windows.Forms;

namespace lab7_winforms_
{
    public partial class LoginForm : Form
    {
        readonly MainForm mainForm;

        public LoginForm()
        {
            InitializeComponent();
            Encoding.RegisterProvider(CodePagesEncodingProvider.Instance);
            mainForm = new MainForm(this);
        }

        private void LoginButton_Click(object sender, EventArgs e)
        {
            string email = emailTextBox.Text;
            string password = passwordTextBox.Text;

            ImapClient imapClient;
            try
            {
                imapClient = new ImapClient("imap.gmail.com", email, password,
                        AuthMethods.Login, 993, true);
            }
            catch (Exception)
            {
                ErrorLabel.Text = "Incorrect email or password. Please try again.";
                return;
            }

            mainForm.LoadMessages(in imapClient);
            mainForm.Show();
            Hide();
        }

        internal void Clear()
        {
            emailTextBox.Text = "";
            passwordTextBox.Text = "";
            ErrorLabel.Text = "";
        }
    }
}
