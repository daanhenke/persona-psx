# Map model binding

`func_80097BE8` at S2D `0x80097BE8` is a 236-byte routine that maps a TMD model and binds its object records to the current map's model IDs. The first word of the TMD is skipped before `GsMapModelingData`; object records begin after the remaining two header words. IDs come from the map-info row after its effect field and end at 255.

For each ID, the routine stores the current seven-word object record in `g_model_objs`, clears its high address bit, and patches the first primitive word: retain its upper byte, then OR in the object's primitive count. The primitive pointer is reloaded between the mask and OR operations, preserving the original alias-sensitive access order.

A local model-table base reproduces the original register lifetime. The small map-info accessor takes an integer index after the caller's signed-short conversion; this preserves the original ordering of address calculation. No assembly, padding or compiler overrides are used.

Acceptance requires all eleven complete JP1 binary SHA-256 checks. This routine still uses the original PS1 TMD mapping API and fixed-address model data; it does not establish native Linux gameplay.
