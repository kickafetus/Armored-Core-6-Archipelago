from worlds.AutoWorld import World
from worlds.generic.Rules import set_rule
from BaseClasses import MultiWorld

from .locations import CYCLE_NAMES


def _chapter_region(cycle: int, chapter: int) -> str:
    # Must match create_regions in __init__.py.
    prefix = "" if cycle == 0 else f"{CYCLE_NAMES[cycle]} "
    return f"{prefix}Chapter {chapter}"


def set_rules(world: World, multiworld: MultiWorld, player: int) -> None:
    # ── Cycle-access gating (multiworld balance) ───────────────────────────
    # Within a cycle, AC6 has no item-based hard gates (you can clear any
    # mission with default gear), so the region chain is the whole intra-cycle
    # progression model. But the cycle TRANSITIONS must be gated on a real item,
    # or the multiworld solver sees all of NG+/NG++ as reachable from sphere 0
    # and may place other games' early-critical progression behind your NG++
    # finale. So each cycle transition requires its access pass (a progression
    # item create_items adds for multi-cycle modes).
    cycle_pass = {1: "NG+ Access", 2: "NG++ Access"}
    for cyc in range(1, world._num_cycles()):
        entrance = f"{_chapter_region(cyc - 1, 5)} -> {_chapter_region(cyc, 1)}"
        set_rule(
            multiworld.get_entrance(entrance, player),
            lambda state, it=cycle_pass[cyc]: state.has(it, player),
        )

    # ── Arena — sequential rank gates ─────────────────────────────────────
    arena_order = ["F", "E", "D", "C", "B", "A", "S"]
    for prev, cur in zip(arena_order, arena_order[1:]):
        set_rule(
            multiworld.get_location(f"Complete Arena {cur}", player),
            lambda state, p=prev: state.can_reach(
                f"Complete Arena {p}", "Location", player),
        )

    # ── Mercenary ranks — sequential ──────────────────────────────────────
    for i in range(2, 18):
        set_rule(
            multiworld.get_location(f"Reach Mercenary Rank {i}", player),
            lambda state, prev=i - 1: state.can_reach(
                f"Reach Mercenary Rank {prev}", "Location", player
            ),
        )
