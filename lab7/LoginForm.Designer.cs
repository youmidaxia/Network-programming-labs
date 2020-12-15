namespace lab7_winforms_
{
    partial class LoginForm
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
            this.label1 = new System.Windows.Forms.Label();
            this.button1 = new System.Windows.Forms.Button();
            this.emailTextBox = new System.Windows.Forms.TextBox();
            this.passwordTextBox = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.ErrorLabel = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 14F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.label1.Location = new System.Drawing.Point(38, 41);
            this.label1.Margin = new System.Windows.Forms.Padding(0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(74, 29);
            this.label1.TabIndex = 0;
            this.label1.Text = "Email";
            // 
            // button1
            // 
            this.button1.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.button1.Font = new System.Drawing.Font("Microsoft Sans Serif", 14F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.button1.Location = new System.Drawing.Point(41, 243);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(315, 49);
            this.button1.TabIndex = 1;
            this.button1.Text = "Log in";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.LoginButton_Click);
            // 
            // textBox1
            // 
            this.emailTextBox.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.emailTextBox.Font = new System.Drawing.Font("Microsoft Sans Serif", 14F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.emailTextBox.Location = new System.Drawing.Point(41, 73);
            this.emailTextBox.Margin = new System.Windows.Forms.Padding(0);
            this.emailTextBox.Name = "textBox1";
            this.emailTextBox.Size = new System.Drawing.Size(315, 34);
            this.emailTextBox.TabIndex = 2;
            this.emailTextBox.Text = "segatron99@gmail.com";
            // 
            // textBox2
            // 
            this.passwordTextBox.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.passwordTextBox.Font = new System.Drawing.Font("Microsoft Sans Serif", 14F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.passwordTextBox.Location = new System.Drawing.Point(41, 143);
            this.passwordTextBox.Name = "textBox2";
            this.passwordTextBox.PasswordChar = '*';
            this.passwordTextBox.Size = new System.Drawing.Size(315, 34);
            this.passwordTextBox.TabIndex = 4;
            this.passwordTextBox.Text = "Exp(i*Pi)+1=0";
            // 
            // label2
            // 
            this.label2.Anchor = System.Windows.Forms.AnchorStyles.None;
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("Microsoft Sans Serif", 14F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.label2.Location = new System.Drawing.Point(38, 111);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(120, 29);
            this.label2.TabIndex = 3;
            this.label2.Text = "Password";
            // 
            // ErrorLabel
            // 
            this.ErrorLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.ErrorLabel.ForeColor = System.Drawing.Color.Firebrick;
            this.ErrorLabel.Location = new System.Drawing.Point(41, 184);
            this.ErrorLabel.Name = "ErrorLabel";
            this.ErrorLabel.Size = new System.Drawing.Size(315, 56);
            this.ErrorLabel.TabIndex = 5;
            // 
            // LoginForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(395, 333);
            this.Controls.Add(this.ErrorLabel);
            this.Controls.Add(this.passwordTextBox);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.emailTextBox);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.label1);
            this.Name = "LoginForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Login to your mailbox";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.TextBox emailTextBox;
        private System.Windows.Forms.TextBox passwordTextBox;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label ErrorLabel;
    }
}

