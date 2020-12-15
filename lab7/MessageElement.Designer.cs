namespace lab7_winforms_
{
    partial class MessageElement
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

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.messageSubjectLabel = new System.Windows.Forms.Label();
            this.messageFromLabel = new System.Windows.Forms.Label();
            this.dateTimeLabel = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // messageSubjectLabel
            // 
            this.messageSubjectLabel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.messageSubjectLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 16F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.messageSubjectLabel.Location = new System.Drawing.Point(78, 32);
            this.messageSubjectLabel.Margin = new System.Windows.Forms.Padding(0);
            this.messageSubjectLabel.Name = "messageSubjectLabel";
            this.messageSubjectLabel.Size = new System.Drawing.Size(320, 66);
            this.messageSubjectLabel.TabIndex = 0;
            this.messageSubjectLabel.Text = " dsfg sggdfgfdg gsgfg dfsg dsg sfg";
            // 
            // messageFromLabel
            // 
            this.messageFromLabel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.messageFromLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.messageFromLabel.Location = new System.Drawing.Point(79, 0);
            this.messageFromLabel.Margin = new System.Windows.Forms.Padding(0);
            this.messageFromLabel.Name = "messageFromLabel";
            this.messageFromLabel.Size = new System.Drawing.Size(319, 32);
            this.messageFromLabel.TabIndex = 1;
            this.messageFromLabel.Text = "Some text";
            this.messageFromLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // dateTimeLabel
            // 
            this.dateTimeLabel.Dock = System.Windows.Forms.DockStyle.Left;
            this.dateTimeLabel.Location = new System.Drawing.Point(0, 0);
            this.dateTimeLabel.Name = "dateTimeLabel";
            this.dateTimeLabel.Size = new System.Drawing.Size(76, 98);
            this.dateTimeLabel.TabIndex = 2;
            this.dateTimeLabel.Text = "12:45\r\n10/05/19";
            this.dateTimeLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // MessageElement
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.Controls.Add(this.dateTimeLabel);
            this.Controls.Add(this.messageSubjectLabel);
            this.Controls.Add(this.messageFromLabel);
            this.Cursor = System.Windows.Forms.Cursors.Hand;
            this.Name = "MessageElement";
            this.Size = new System.Drawing.Size(398, 98);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Label messageSubjectLabel;
        private System.Windows.Forms.Label messageFromLabel;
        private System.Windows.Forms.Label dateTimeLabel;
    }
}
