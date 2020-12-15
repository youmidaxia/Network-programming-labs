namespace lab7_winforms_
{
    partial class MainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.messagesPanel = new System.Windows.Forms.Panel();
            this.LoadMoreButton = new System.Windows.Forms.Button();
            this.LogoutButton = new System.Windows.Forms.Button();
            this.messageElement2 = new lab7_winforms_.MessageElement();
            this.messagesPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // messagesPanel
            // 
            this.messagesPanel.AutoScroll = true;
            this.messagesPanel.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.messagesPanel.Controls.Add(this.messageElement2);
            this.messagesPanel.Controls.Add(this.LoadMoreButton);
            this.messagesPanel.Location = new System.Drawing.Point(92, 62);
            this.messagesPanel.Name = "messagesPanel";
            this.messagesPanel.Size = new System.Drawing.Size(564, 433);
            this.messagesPanel.TabIndex = 0;
            // 
            // LoadMoreButton
            // 
            this.LoadMoreButton.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.LoadMoreButton.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.LoadMoreButton.Location = new System.Drawing.Point(0, 393);
            this.LoadMoreButton.Name = "LoadMoreButton";
            this.LoadMoreButton.Size = new System.Drawing.Size(562, 38);
            this.LoadMoreButton.TabIndex = 1;
            this.LoadMoreButton.Text = "Load more";
            this.LoadMoreButton.UseVisualStyleBackColor = true;
            this.LoadMoreButton.Click += new System.EventHandler(this.LoadMoreButton_Click);
            // 
            // LogoutButton
            // 
            this.LogoutButton.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.LogoutButton.Location = new System.Drawing.Point(524, 12);
            this.LogoutButton.Name = "LogoutButton";
            this.LogoutButton.Size = new System.Drawing.Size(132, 44);
            this.LogoutButton.TabIndex = 1;
            this.LogoutButton.Text = "Log out ->";
            this.LogoutButton.UseVisualStyleBackColor = true;
            this.LogoutButton.Click += new System.EventHandler(this.LogoutButton_Click);
            // 
            // messageElement2
            // 
            this.messageElement2.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.messageElement2.Cursor = System.Windows.Forms.Cursors.Hand;
            this.messageElement2.Dock = System.Windows.Forms.DockStyle.Top;
            this.messageElement2.Location = new System.Drawing.Point(0, 0);
            this.messageElement2.Name = "messageElement2";
            this.messageElement2.Size = new System.Drawing.Size(562, 98);
            this.messageElement2.TabIndex = 2;
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(662, 507);
            this.Controls.Add(this.LogoutButton);
            this.Controls.Add(this.messagesPanel);
            this.Name = "MainForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Inbox";
            this.messagesPanel.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Panel messagesPanel;
        private System.Windows.Forms.Button LogoutButton;
        private System.Windows.Forms.Button LoadMoreButton;
        private MessageElement messageElement2;
    }
}