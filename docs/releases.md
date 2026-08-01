# Binary releases

GitHub Releases provide one archive for each supported board. Releases are
built from stable tags named `vMAJOR.MINOR.PATCH`; the tag is the only project
version source. The exact board assets are `pinebook-pro`, `genbook`,
`roc3566`, `yy3568`, and `rock3a`, plus top-level `SHA256SUMS`.

## Downloading and verifying

Download `SHA256SUMS` and the archive for the board, then verify the download
before extracting it:

```sh
sha256sum -c SHA256SUMS
tar -xJf rk-v1.2.3-roc3566.tar.xz
```

The checksum command reports missing archives if only one board archive was
downloaded. That is harmless as long as the selected archive reports `OK`; to
verify just one file, use:

```sh
grep 'roc3566.tar.xz' SHA256SUMS | sha256sum -c -
```

Replace `v1.2.3` with the downloaded release version. Each archive has one
versioned top-level directory and another `SHA256SUMS` covering its extracted
contents.

## Bundle contents

Every bundle contains the project README and license, board documentation,
`BUILD-INFO.txt`, and these directories:

- `firmware/`: a firmware-only developer base and the same firmware with the
  demo payload appended. The firmware-only `.bin` is not standalone; append a
  compatible FUEFI payload before loading it.
- `images/`: complete RKNS images to write to an SD card. Files beginning with
  `demo_`, plus the historically named `genbook_demo.img`, start the included
  demo payload and are standalone bring-up tests. The other `.img` contains
  firmware only, expects an appended payload, and halts with
  `Bad payload magic` when run unchanged.
- `loaders/`: the board's DDR loader. RK356x bundles also carry the shared
  USB-plug blob for the optional `xrock maskrom ... --rc4-off` flow.

The YY3568 and ROCK 3A bundles additionally carry their own optional
BL31/U-Boot chainloader, SD image, raw eMMC ID block, SPI-NOR image, guarded
install/restore scripts, verified BL31 ELF and provenance, manifest, and
patched corresponding-source U-Boot archive. The installer requires separately
installed xrock and rkdeveloptool; no host executable is bundled. Each archive
contains only its own board's manifest, U-Boot source, and artifacts.

The RK356x archives include Rockchip's binary license and the pinned rkbin
provenance. Write an `.img` to the whole SD device with a trusted imaging tool,
carefully checking the destination because writing an image replaces the
device's existing partition table and data.

SD is an image-delivery medium in the normal/demo flow: Rockchip BootROM reads
the image before the firmware starts. Its presence does not mean the
bare-metal firmware can mount the SD card. See
[Firmware images, payloads, and device trees](payloads.md) for the execution
model and custom-payload packaging example.

## Using an RK3568 chainloader bundle

The YY3568 and ROCK 3A archives keep persistent-media files under `chainload/`
and the guarded utility under `install/`. After checking the internal
`SHA256SUMS`, a RAM-only first test can use
`chainload/uboot_<board>.bin` with an installed MaskROM loader. Persistent
installation uses the packaged utility and separately installed
xrock/rkdeveloptool:

```sh
./install/flash-chainload.sh flash yy3568 emmc \
  /absolute/path/yy3568-emmc-backup yy3568:emmc

./install/flash-chainload.sh flash yy3568 spi-nor \
  /absolute/path/yy3568-spi-backup yy3568:spi-nor
```

Use `rock3a` in all four board-qualified positions for a ROCK 3A archive.
Cross-board backup restoration is rejected using the board name, selected
medium, manifest hash, detected capacity, and saved range.

Do not pass the SD image to the eMMC installer: eMMC expects the raw
`uboot_<board>_idbloader.img` at LBA `0x40`, while
`uboot_<board>.img` already contains 64 leading sectors for whole-SD imaging.
SPI NOR uses `uboot_<board>_spi.img` at offset zero. The utility resolves these
files from the bundle and selects the correct artifact from its manifest.

Read `chainloading.md` in the archive before writing persistent storage. In
particular, valid SPI firmware has priority over eMMC, SD, and ordinary USB
fallback.

After U-Boot starts, its board manifest applies the separate OS discovery
order NVMe, removable SD, USB mass storage, then eMMC. This is independent of
which medium supplied the chainloader.

Linux host programs (`rock.out` and `makeboot.out`) and the partial Orange Pi 5
target are intentionally not release assets. They can still be built from the
source tree.

## Creating a release

### Triggering the workflow

The sole release trigger is a pushed Git tag whose name is exactly
`vMAJOR.MINOR.PATCH`. Prerelease suffixes, ordinary `master` pushes, and pull
requests do not trigger a release. The workflow intentionally has no manual
`workflow_dispatch` entry point.

Before tagging, merge the intended commit into `master` and confirm that the
`build and check` workflow succeeds. Create and push one annotated stable tag:

```sh
git switch master
git pull --ff-only
git status --short
git tag -a v1.2.3 -m 'rk v1.2.3'
git push origin v1.2.3
```

Push only the intended tag rather than using `git push --tags`. The tag push
starts the `release` workflow automatically. Follow it under **Actions ->
release** in the GitHub web interface.

Do not use **Releases -> Draft a new release** to trigger CI. That interface
creates a GitHub Release object before the workflow starts, so the immutability
guard treats it as an existing release. The web interface is used to monitor or
retry the workflow; Git creates the release tag.

The release workflow verifies the tag format and confirms its commit is
contained in `origin/master`. It then runs the full test suite, creates
reproducible archives, checks their file allowlists and hashes, uploads an
unpublished draft, verifies the remote assets, and finally publishes it as the
latest release. A failed build never creates a public release, and an existing
published release is never overwritten. If a tag workflow is retried after its
release was published, CI downloads the existing assets, checks their exact
name/size allowlist and SHA-256 manifest against the clean rebuild, and succeeds
only when they match. A different or incomplete published release remains a
hard failure. Drafts use GitHub's temporary `untagged-*` web URL; CI discovers
their numeric release ID through GitHub's authenticated GraphQL pending-tag
lookup before checking assets. This follows the GitHub CLI's draft-resolution
path; the REST endpoint that looks up a release by tag is explicitly limited to
published releases. Do not move or reuse a published tag.

### Retrying a release

For a transient runner or network failure, open **Actions -> release**, select
the failed tag run, and choose **Re-run jobs -> Re-run failed jobs**. A retry
uses the workflow and repository contents from the tagged commit:

- If the failed run left an unpublished draft for that tag, the workflow removes
  that draft and recreates it before uploading the verified assets.
- If the release is already public and its six assets match the reproducible
  rebuild, the retry succeeds without modifying the release.
- If a public release is empty, incomplete, or different, the retry fails. Do
  not upload over it automatically; investigate how it was created and use a
  new SemVer tag when it may already have been consumed.
- If fixing the failure required a repository commit, rerunning the old tag
  cannot use that fix. Merge the correction into `master` and push the next
  SemVer tag instead.

There is no **Run workflow** button for `release.yml`; that is intentional.
Pushing a new valid tag is the only way to start a new release run.

`master` pushes, documentation deployment, and tag releases use the repository
variable `RK_RUNNER_LABELS`. If the variable is absent, workflows select the
repository runner with `[self-hosted, Linux, X64]`. To use a GitHub-hosted
runner instead, set the variable to `["ubuntu-latest"]`. Pull requests do not
trigger a workflow. A self-hosted Debian account must provide noninteractive
`sudo apt-get`, outbound GitHub access, and enough disk space for two isolated
U-Boot source builds. Release dependencies include the GitHub `gh` CLI; the
same dependency installation remains compatible with `ubuntu-latest`.

The same distribution can be inspected locally after building the toolchain:

```sh
make check
make chainload BOARD=yy3568
make chainload BOARD=rock3a
make release-dist VERSION=v1.2.3
```

`DIST_DIR` defaults to `dist`. The packager refuses a non-empty destination;
use a new directory instead of overwriting an earlier distribution.
