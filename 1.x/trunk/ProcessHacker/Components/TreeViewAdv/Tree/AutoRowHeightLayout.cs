using System;
using System.Collections.Generic;
using System.Drawing;
using Aga.Controls.Tree.NodeControls;

namespace Aga.Controls.Tree
{
	public class AutoRowHeightLayout: IRowLayout
	{
		private DrawContext _measureContext;
		private readonly TreeViewAdv _treeView;
		private readonly List<Rectangle> _rowCache;

		public AutoRowHeightLayout(TreeViewAdv treeView, int rowHeight)
		{
			_rowCache = new List<Rectangle>();
			_treeView = treeView;
			PreferredRowHeight = rowHeight;
			_measureContext = new DrawContext();
			_measureContext.Graphics = Graphics.FromImage(new Bitmap(1, 1));
		}

		private int _rowHeight;
		public int PreferredRowHeight
		{
			get { return _rowHeight; }
			set { _rowHeight = value; }
		}


		public int PageRowCount
		{
			get 
			{
				if (_treeView.RowCount == 0)
					return 0;
			    int pageHeight = this._treeView.DisplayRectangle.Height - this._treeView.ColumnHeaderHeight;
			    int y = 0;
			    for (int i = this._treeView.RowCount - 1; i >= 0; i--)
			    {
			        y += this.GetRowHeight(i);
			        if (y > pageHeight)
			            return Math.Max(0, this._treeView.RowCount - 1 - i);
			    }
			    return this._treeView.RowCount;
			}
		}

		public int CurrentPageSize
		{
			get
			{
				if (_treeView.RowCount == 0)
					return 0;
			    int pageHeight = this._treeView.DisplayRectangle.Height - this._treeView.ColumnHeaderHeight;
			    int y = 0;
			    for (int i = this._treeView.FirstVisibleRow; i < this._treeView.RowCount; i++)
			    {
			        y += this.GetRowHeight(i);
			        if (y > pageHeight)
			            return Math.Max(0, i - this._treeView.FirstVisibleRow);
			    }
			    return Math.Max(0, this._treeView.RowCount - this._treeView.FirstVisibleRow);
			}
		}

		public Rectangle GetRowBounds(int rowNo)
		{
			if (rowNo >= _rowCache.Count)
			{
				int count = _rowCache.Count;
				int y = count > 0 ? _rowCache[count - 1].Bottom : 0;
				for (int i = count; i <= rowNo; i++)
				{
					int height = GetRowHeight(i);
					_rowCache.Add(new Rectangle(0, y, 0, height));
					y += height;
				}
				if (rowNo < _rowCache.Count - 1)
					return Rectangle.Empty;
			}
			if (rowNo >= 0 && rowNo < _rowCache.Count)
				return _rowCache[rowNo];
		    return Rectangle.Empty;
		}

		private int GetRowHeight(int rowNo)
		{
		    if (rowNo < _treeView.RowMap.Count)
			{
				TreeNodeAdv node = _treeView.RowMap[rowNo];
				if (node.Height == null)
				{
					int res = 0;
					_measureContext.Font = _treeView.Font;
					foreach (NodeControl nc in _treeView.NodeControls)
					{
						int h = nc.GetActualSize(node, _measureContext).Height;
						if (h > res)
							res = h;
					}
					node.Height = res;
				}
				return node.Height.Value;
			}
		    return 0;
		}

	    public int GetRowAt(Point point)
		{
			int py = point.Y - _treeView.ColumnHeaderHeight;
			int y = 0;
			for (int i = _treeView.FirstVisibleRow; i < _treeView.RowCount; i++)
			{
				int h = GetRowHeight(i);
				if (py >= y && py < y + h)
					return i;
			    y += h;
			}
			return -1;
		}

		public int GetFirstRow(int lastPageRow)
		{
			int pageHeight = _treeView.DisplayRectangle.Height - _treeView.ColumnHeaderHeight;
			int y = 0;
			for (int i = lastPageRow; i >= 0; i--)
			{
				y += GetRowHeight(i);
				if (y > pageHeight)
					return Math.Max(0, i + 1);
			}
			return 0;
		}

		public void ClearCache()
		{
			_rowCache.Clear();
		}
	}
}
