using Ropufu.LeytePond.Bridge;
using System;
using System.Windows;
using System.Windows.Input;

namespace Ropufu.LeytePond
{
    /// <summary>
    /// Interaction logic for SkillsWindow.xaml
    /// </summary>
    public partial class SkillsWindow : Window
    {
        private SkillMap skills = null;

        public SkillsWindow(ArmyDecorator decorator = null, UnitType general = null)
        {
            if (Object.ReferenceEquals(decorator, null)) decorator = new ArmyDecorator();

            this.skills = decorator.Skills;
            this.InitializeComponent();
            if (!general.IsNull()) this.generalsBox.SelectedItem = general;
        }

        public SkillMap Skills => this.skills;

        protected override void OnPreviewKeyDown(KeyEventArgs e)
        {
            switch (e.Key)
            {
                case Key.Escape:
                    this.Close();
                    e.Handled = true;
                    break;
            }
            base.OnPreviewKeyDown(e);
        }
    }
}
