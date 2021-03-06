﻿namespace ProcessHacker.Components
{
    partial class TokenGroupsList
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
            this.listGroups = new ProcessHacker.Components.ExtendedListView();
            this.columnGroupName = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnFlags = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.SuspendLayout();
            // 
            // listGroups
            // 
            this.listGroups.AllowColumnReorder = true;
            this.listGroups.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnGroupName,
            this.columnFlags});
            this.listGroups.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listGroups.DoubleClickChecks = true;
            this.listGroups.FullRowSelect = true;
            this.listGroups.Location = new System.Drawing.Point(0, 0);
            this.listGroups.Name = "listGroups";
            this.listGroups.ShowItemToolTips = true;
            this.listGroups.Size = new System.Drawing.Size(416, 397);
            this.listGroups.Sorting = System.Windows.Forms.SortOrder.Ascending;
            this.listGroups.TabIndex = 4;
            this.listGroups.UseCompatibleStateImageBehavior = false;
            this.listGroups.View = System.Windows.Forms.View.Details;
            // 
            // columnGroupName
            // 
            this.columnGroupName.Text = "Name";
            this.columnGroupName.Width = 200;
            // 
            // columnFlags
            // 
            this.columnFlags.Text = "Flags";
            this.columnFlags.Width = 180;
            // 
            // TokenGroupsList
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.WhiteSmoke;
            this.Controls.Add(this.listGroups);
            this.Name = "TokenGroupsList";
            this.Size = new System.Drawing.Size(416, 397);
            this.ResumeLayout(false);

        }

        #endregion

        private ExtendedListView listGroups;
        private System.Windows.Forms.ColumnHeader columnGroupName;
        private System.Windows.Forms.ColumnHeader columnFlags;
    }
}
