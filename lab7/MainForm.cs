using AE.Net.Mail;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Windows.Forms;

namespace lab7_winforms_
{
    public partial class MainForm : Form
    {
        readonly LoginForm loginForm;
        ImapClient imapClient;
        const int numToLoad = 20;
        int currentPage = 1;

        public MainForm()
        {
            InitializeComponent();
        }

        public MainForm(LoginForm loginForm) : this()
        {
            this.loginForm = loginForm;
        }

        internal void LoadMessages(in ImapClient imapClient)
        {
            this.imapClient = imapClient;
            messagesPanel.Controls.Clear();

            currentPage = 1;
            GetMailMessages();
            messagesPanel.Controls[messagesPanel.Controls.Count - 1].Select();
            messagesPanel.Controls.Add(LoadMoreButton);
        }

        private void GetMailMessages()
        {
            imapClient.SelectMailbox("INBOX");
            var messagesNum = imapClient.GetMessageCount();
            var startNum = messagesNum - 1 - (currentPage - 1) * numToLoad;
            var lastNum = messagesNum - currentPage * numToLoad;
            var messages = imapClient.GetMessages(startNum, lastNum);

            var controls = messagesPanel.Controls.Cast<Control>().ToArray();
            var newControls = new List<Control>();
            foreach (MailMessage mail in messages)
            {
                var displayName = mail.From.DisplayName;
                if (displayName == "") displayName = mail.From.Address;
                var messageElement = new MessageElement(mail.Subject, displayName, mail.Date)
                {
                    Dock = DockStyle.Top,
                    Tag = mail
                };
                newControls.Add(messageElement);
            }
            messagesPanel.Controls.Clear();
            newControls.AddRange(controls);
            messagesPanel.Controls.AddRange(newControls.ToArray());
        }

        private void LogoutButton_Click(object sender, EventArgs e)
        {
            imapClient.Logout();
            imapClient.Dispose();

            loginForm.Clear();
            loginForm.Show();
            Hide();
        }

        private void LoadMoreButton_Click(object sender, EventArgs e)
        {           
            currentPage++;

            var vScrollPos = messagesPanel.VerticalScroll.Value;
            messagesPanel.Controls.Remove(LoadMoreButton);
            GetMailMessages();
            messagesPanel.Controls.Add(LoadMoreButton);
            messagesPanel.AutoScrollPosition = new Point(messagesPanel.AutoScrollPosition.X, vScrollPos);
        }
    }
}
