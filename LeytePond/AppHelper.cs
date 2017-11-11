using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Media;

namespace Ropufu.LeytePond
{
    static class AppHelper
    {
        public static DependencyObject FindVisualChild(this DependencyObject item, Predicate<DependencyObject> predicate)
        {
            if (Object.ReferenceEquals(item, null)) return null;

            if (predicate(item)) return item;

            var count = VisualTreeHelper.GetChildrenCount(item);
            for (var i = 0; i < count; i++)
            {
                var result = VisualTreeHelper.GetChild(item, i).FindVisualChild(predicate);
                if (!Object.ReferenceEquals(result, null)) return result;
            }
            return null;
        }

        public static void ForVisualChildren(this DependencyObject item, Predicate<DependencyObject> predicate, Action<DependencyObject> action)
        {
            if (Object.ReferenceEquals(item, null)) return;

            if (predicate(item)) action(item);

            var count = VisualTreeHelper.GetChildrenCount(item);
            for (var i = 0; i < count; i++) VisualTreeHelper.GetChild(item, i).ForVisualChildren(predicate, action);
        }
    }
}
