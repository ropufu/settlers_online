using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Ropufu.UpdaterApp
{
    public class InstructionEventArgs : EventArgs
    {
        private IInstruction instruction; 

        /// <exception cref="ArgumentNullException"></exception>
        public InstructionEventArgs(IInstruction instruction)
        {
            if (instruction.IsNull()) throw new ArgumentNullException(nameof(instruction));
            this.instruction = instruction;
        }

        public IInstruction Instruction => this.instruction;
    }
}
