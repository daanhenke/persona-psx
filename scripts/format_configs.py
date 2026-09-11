#!/usr/bin/env python3
"""Put every sym file in a config directory into the project's house order.

Sorts each `sym.<target>.txt` by address, upper-cases the hex, and re-inserts
the subsegment comments from the matching `<target>.yaml`, so a symbol's file
position says which unit it belongs to.

Run it before committing a change to a sym file - the diff is otherwise a
reshuffle on top of the real edit.

  usage: format_configs.py [region ...]

Regions default to every directory under `configs/` that holds a `sym.*.txt`,
so this needs no arguments and nothing hardcoded about which versions exist.
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CONFIGS = ROOT / "configs"


def regions(argv):
    """The config directories to format: those named, or all that have syms."""
    if argv:
        out = []
        for name in argv:
            path = Path(name)
            if not path.is_dir():
                path = CONFIGS / name
            if not path.is_dir():
                sys.exit("no such config directory: %s" % name)
            out.append(path)
        return out
    found = sorted(p for p in CONFIGS.iterdir()
                   if p.is_dir() and any(p.glob("sym.*.txt")))
    if not found:
        sys.exit("no sym files under %s" % CONFIGS)
    return found


def pairs(region):
    """Each sym file with the yaml that names its subsegments."""
    syms = {p.stem[len("sym."):]: p for p in sorted(region.glob("sym.*.txt"))}
    yamls = {p.stem: p for p in sorted(region.glob("*.yaml"))}
    return [(syms[k], yamls[k]) for k in sorted(syms) if k in yamls]


# =============================================================================
# YAML PARSING AND COMMENT GENERATION
# =============================================================================


def parse_yaml_for_comments(yaml_file):
    """Parse YAML file and generate comments."""

    SECTION_TYPES = {
        'rodata': ['.rodata', 'rodata', '.rdata'],
        'text': ['.text', 'asm', 'hasm', 'asmtu', 'c', 'cpp'],
        'data': ['.data', 'data'],
        'sdata': ['.sdata', 'sdata'],
        'bss': ['.bss', 'bss'],
        'sbss': ['.sbss', 'sbss'],
        'pad': ['pad']  # Special handling - no section header
    }

    with open(yaml_file, 'r') as f:
        content = f.read()

    # Simple approach - find first start: and vram: lines
    lines = content.split('\n')
    ram_start = None
    vram_start = None

    current_block = []

    for line in lines:
        stripped = line.strip()

        # Detect the start of a new segment block
        if stripped.startswith('- name:'):
            if current_block:
                # Check previous block before starting new one
                block_text = "\n".join(current_block)
                if 'start:' in block_text and 'vram:' in block_text:
                    for blk_line in current_block:
                        blk_line = blk_line.strip()
                        if blk_line.startswith('start:'):
                            ram_start = int(blk_line.split('0x')[1], 16) if '0x' in blk_line else int(
                                blk_line.split(':')[1].strip())
                        elif blk_line.startswith('vram:'):
                            vram_start = int(blk_line.split('0x')[1], 16) if '0x' in blk_line else int(
                                blk_line.split(':')[1].strip())
                    break  # Found the correct block, stop searching
            current_block = [stripped]  # Start new block
        elif stripped.startswith('segments:'):
            continue
        elif current_block is not None:
            current_block.append(stripped)

    # In case the last block was the correct one
    if ram_start is None or vram_start is None:
        block_text = "\n".join(current_block)
        if 'start:' in block_text and 'vram:' in block_text:
            for blk_line in current_block:
                blk_line = blk_line.strip()
                if blk_line.startswith('start:'):
                    ram_start = int(blk_line.split('0x')[1], 16) if '0x' in blk_line else int(
                        blk_line.split(':')[1].strip())
                elif blk_line.startswith('vram:'):
                    vram_start = int(blk_line.split('0x')[1], 16) if '0x' in blk_line else int(
                        blk_line.split(':')[1].strip())

    # if ram_start is not None and vram_start is not None:
        # print(f"DEBUG ADDRESSES: ram_start=0x{ram_start:X}, vram_start=0x{vram_start:X}")
    # else:
        # print("Could not find a segment with both start and vram.")

    # Create section headers (exclude pad)
    section_map = {
        name: [
            "//" + "=" * 48,
            f"//=== .{name} section",
            "//" + "=" * 48
        ]
        for name in SECTION_TYPES.keys() if name != 'pad'
    }

    # Parse only subsegment lines (starting with "      - [0x")
    subsegment_map = {}

    for line in lines:
        line = line.strip()
        if line.startswith('- [0x') and ', ' in line:
            # Strip everything after the first closing bracket (including comments)
            line = line.split(']', 1)[0] + ']'

            # Parse: "- [0x1234, stuff, more stuff]"
            line = line[2:]  # Remove "- "
            line = line[1:-1]  # Remove [ ]
            parts = [part.strip() for part in line.split(',')]

            ram = int(parts[0], 16)  # First part is always the address
            vram = vram_start - ram_start + ram
            segment_info = ', '.join(parts[1:])  # Everything else

            # print(f"DEBUG CALC: ram={parts[0]} -> vram=0x{vram_start:X} - 0x{ram_start:X} + {parts[0]} = 0x{vram:X}")

            # Clean up segment parts for section detection
            clean_parts = [part.strip() for part in parts[1:]]

            if clean_parts[0] == 'lib' or clean_parts[0] == 'o':
                section_type = clean_parts[-1]
                section_name = None
                for name, types in SECTION_TYPES.items():
                    if section_type in types:
                        section_name = name if name != 'pad' else None
                        break
                if not section_name:
                    section_name = 'text'
            else:
                section_type = clean_parts[0]
                section_name = None
                for name, types in SECTION_TYPES.items():
                    if section_type in types:
                        section_name = name if name != 'pad' else None
                        break

            if section_name:
                info_line = f"PA: 0x{ram:06X}   VA: 0x{vram:08X}   subsegment: {segment_info}"
                separator = "//----" + "-" * len(info_line)
                comment_lines = [separator, f"//--- {info_line}", separator]
                subsegment_map[vram] = (section_name, comment_lines)

    return section_map, subsegment_map


# =============================================================================
# TEXT PROCESSING AND FORMATTING
# =============================================================================

def clean_yaml_comments_from_txt(txt_lines):
    """Remove all YAML-generated comments and decorative lines"""
    cleaned = []

    for txt_line in txt_lines:

        # Skip empty lines
        if not txt_line:
            continue

        # Skip all YAML-generated content
        if (txt_line.startswith("//---") or
            txt_line.startswith("//===")):
            continue

        cleaned.append(txt_line)

    return cleaned


def convert_hex_to_uppercase(text):
    """Convert hex values to uppercase"""
    lines = text.split('\n')
    result = []

    for line in lines:
        if "0x" in line:
            # Find and convert hex values
            hex_match = re.search(r'0x[0-9a-fA-F]+', line)
            if hex_match:
                try:
                    hex_value = int(hex_match.group(), 16)
                    uppercase_hex = f"0x{hex_value:X}"
                    line = line.replace(hex_match.group(), uppercase_hex)
                except ValueError:
                    pass
        result.append(line)

    return '\n'.join(result)


def sort_and_format_with_yaml_comments(input_text, section_map, subsegment_map):
    """Sort variables by address and insert YAML comments at the right places"""
    import re

    lines = input_text.strip().split('\n')

    # Parse txt file into variable groups
    variables = []
    current_comments = []

    for line in lines:
        if not line.strip() or line.strip().startswith('//'):
            current_comments.append(line)
        else:
            # Variable line - extract address
            hex_match = re.search(r'0x[0-9a-fA-F]+', line)
            address = int(hex_match.group(), 16) if hex_match else 0

            variables.append({
                'address': address,
                'variable_line': line,
                'original_comments': current_comments[:]
            })
            current_comments = []

    # Calculate alignment for variables
    max_var_length = 0
    for var in variables:
        if '=' in var['variable_line']:
            var_part = var['variable_line'].split('=')[0].rstrip()
            max_var_length = max(max_var_length, len(var_part))

    # Sort variables by address
    variables.sort(key=lambda x: x['address'])

    # Determine which subsegments are non-empty (have at least one variable within their address range)
    sorted_subseg_addrs = sorted(subsegment_map.keys())
    non_empty_subsegments = set()
    for i, addr in enumerate(sorted_subseg_addrs):
        next_addr = sorted_subseg_addrs[i + 1] if i + 1 < len(sorted_subseg_addrs) else float('inf')
        if any(addr <= v['address'] < next_addr for v in variables):
            non_empty_subsegments.add(addr)

    # Build output by processing variables in order and inserting subsegments when needed
    output_lines = []
    seen_sections = set()
    used_subsegments = set()

    for var in variables:
        address = var['address']

        # Find all subsegments that should appear before this variable
        subsegments_to_add = []
        for sub_addr, (section_name, subseg_comments) in subsegment_map.items():
            if sub_addr <= address and sub_addr not in used_subsegments:
                subsegments_to_add.append((sub_addr, section_name, subseg_comments))
                used_subsegments.add(sub_addr)

        # Sort subsegments by address and add them (skip empty subsegments)
        for sub_addr, section_name, subseg_comments in sorted(subsegments_to_add):
            if sub_addr not in non_empty_subsegments:
                continue

            # print(f"DEBUG FORMAT: Adding subsegment 0x{sub_addr:08X} before variable 0x{address:08X}")

            # Add section header if not seen before
            if section_name not in seen_sections:
                output_lines.extend(['', '', ''])  # Spacing before section
                output_lines.extend(section_map[section_name])
                output_lines.append('')  # Spacing after section
                seen_sections.add(section_name)

            # Add subsegment comment
            output_lines.append('')  # Spacing before subsegment
            output_lines.extend(subseg_comments)

        # Add original comments
        for comment in var['original_comments']:
            output_lines.append(comment)

        # Add aligned variable line
        line = var['variable_line']
        if '=' in line:
            parts = line.split('=', 1)
            var_part = parts[0].rstrip()
            value_part = parts[1]
            aligned_line = var_part.ljust(max_var_length) + ' =' + value_part
            output_lines.append(aligned_line)
        else:
            output_lines.append(line)

    # Add newline to file end
    output_lines.append('')

    return '\n'.join(output_lines)

def main():
    total = 0
    for region in regions(sys.argv[1:]):
        matched = pairs(region)
        print("%s: %d sym files" % (region.name, len(matched)))
        for sym_file, yaml_file in matched:
            text = sym_file.read_text(encoding="utf-8")
            section_map, subsegment_map = parse_yaml_for_comments(yaml_file)

            lines = clean_yaml_comments_from_txt(text.split("\n"))
            cleaned = convert_hex_to_uppercase("\n".join(lines))
            formatted = sort_and_format_with_yaml_comments(
                cleaned, section_map, subsegment_map)

            if formatted != text:
                sym_file.write_text(formatted, encoding="utf-8")
                print("  %-20s rewritten (%d subsegments)"
                      % (sym_file.name, len(subsegment_map)))
                total += 1
            else:
                print("  %-20s already formatted" % sym_file.name)
    print("%d file(s) rewritten" % total)


if __name__ == "__main__":
    main()
