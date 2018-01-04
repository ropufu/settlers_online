using System;
using System.IO;

namespace Ropufu.UpdaterApp
{
    public interface IInstruction
    {
        /// <summary>
        /// Reads the instruction from a stream.
        /// </summary>
        Boolean ReadFrom(StreamReader reader);

        /// <summary>
        /// Writes the instruction to a stream.
        /// </summary>
        Boolean WriteTo(StreamWriter writer);

        /// <summary>
        /// Tries to executes the instruction.
        /// </summary>
        /// <returns></returns>
        Boolean TryExecute();

        /// <summary>
        /// Tries to rollback the instruction.
        /// </summary>
        /// <returns></returns>
        Boolean Rollback();
    }
}
