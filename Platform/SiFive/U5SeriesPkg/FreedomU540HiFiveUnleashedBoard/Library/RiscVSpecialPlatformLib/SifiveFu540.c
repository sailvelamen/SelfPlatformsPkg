/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2020 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>

   Copyright (c) 2021, Hewlett Packard Enterprise Development LP. All rights reserved.<BR>

   SPDX-License-Identifier: BSD-2-Clause-Patent
 */
#include <libfdt.h>
#include <Library/RiscVSpecialPlatformLib.h>
#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/fdt/fdt_fixup.h>

static u64 sifive_fu540_tlbr_flush_limit(const struct fdt_match *match)
{
  /*
   * The sfence.vma by virtual address does not work on
   * SiFive FU540 so we return remote TLB flush limit as zero.
   */
  return 0;
}

static int sifive_fu540_fdt_fixup(void *fdt, const struct fdt_match *match)
{
  /*
   * SiFive Freedom U540 has an erratum that prevents S-mode software
   * to access a PMP protected region using 1GB page table mapping, so
   * always add the no-map attribute on this platform.
   */
  fdt_reserved_memory_nomap_fixup(fdt);

  return 0;
}

static u32 selected_hartid = 1;

static bool sifive_fu540_cold_boot_allowed(u32 hartid, const struct fdt_match *match)
{
  if (selected_hartid != -1)
    return (selected_hartid == hartid);

return true;
}

static const struct fdt_match sifive_fu540_match[] = {
  { .compatible = "sifive,fu540" },
  { .compatible = "sifive,fu540g" },
  { .compatible = "sifive,fu540-c000" },
  { .compatible = "sifive,hifive-unleashed-a00" },
  { },
};

const struct platform_override sifive_fu540 = {
  .match_table = sifive_fu540_match,
  .cold_boot_allowed = sifive_fu540_cold_boot_allowed,
  .tlbr_flush_limit = sifive_fu540_tlbr_flush_limit,
  .fdt_fixup = sifive_fu540_fdt_fixup,
};

const struct platform_override *special_platforms[] = {
  &sifive_fu540,
};
INTN NumberOfPlaformsInArray = array_size(special_platforms);
