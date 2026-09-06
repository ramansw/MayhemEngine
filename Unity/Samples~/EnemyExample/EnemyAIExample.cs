using UnityEngine;

namespace MayhemDebugger.Samples
{
    /// <summary>
    /// C# port of the C++ console demo at
    /// modules/MayhemDebugger/samples/enemy_demo/main.cpp -- same scenario
    /// (an enemy that won't attack because the target is just out of range),
    /// same chain key and step names, so the two samples are directly
    /// comparable side by side.
    ///
    /// Setup:
    ///  1. Build the MayhemDebuggerUnityBridge CMake target.
    ///  2. Copy the resulting native library into this project's
    ///     Assets/Plugins/ folder.
    ///  3. Copy MDBG.cs (and, for the editor viewer,
    ///     Editor/MayhemDebuggerWindow.cs) into this project's Assets/.
    ///  4. Drop this component on any GameObject and enter Play Mode.
    ///  5. Open Window > MayhemDebugger to watch the chain update live as
    ///     you move the Target closer / farther in the Inspector.
    /// </summary>
    public class EnemyAIExample : MonoBehaviour
    {
        [Header("Enemy")]
        public float attackRange = 5.0f;
        public float cooldown = 0.0f;
        public bool hasLineOfSight = true;

        [Header("Target (assign any Transform, or leave empty to simulate 'no target')")]
        public Transform target;

        [Header("Simulated target position (used only if Target is unassigned)")]
        public Vector3 simulatedTargetPosition = new Vector3(5.7f, 0f, 0f);

        private void Update()
        {
            TryAttack();
        }

        private void TryAttack()
        {
            using (MDBG.BeginChain("CanAttack"))
            {
                Vector3? targetPosition = target != null ? target.position
                    : (simulatedTargetPosition != Vector3.zero ? (Vector3?)simulatedTargetPosition : null);

                if (!MDBG.Check("TargetAcquired", targetPosition.HasValue))
                {
                    return;
                }

                float distance = Vector3.Distance(transform.position, targetPosition.Value);
                if (!MDBG.Check("InAttackRange", distance <= attackRange,
                        ("distance", distance), ("range", attackRange)))
                {
                    return; // <- same step the C++ sample fails on when the target is 5.7m away with a 5m range
                }

                if (!MDBG.Check("HasLineOfSight", hasLineOfSight))
                {
                    return;
                }

                if (!MDBG.Check("CooldownReady", cooldown <= 0.0f, ("cooldown", cooldown)))
                {
                    return;
                }

                Debug.Log("Enemy attacks!");
                MDBG.Check("AttackExecuted", true);
            }
        }
    }
}
