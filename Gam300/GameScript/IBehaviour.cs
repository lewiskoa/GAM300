using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BoomDotNet
{
    public interface IBehaviour { void OnStart(); void OnUpdate(float dt); }
}
