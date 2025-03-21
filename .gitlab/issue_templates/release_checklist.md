---
name: a.b.c release checklist
about: Create a new issue using this checklist for each release.
---

# Kea Release Checklist

This is thoroughly documented in [the Kea Release Process guide](https://wiki.isc.org/bin/view/QA/KeaReleaseProcess).

## Pre-Release Preparation

Some of those checks and updates can be made before the actual freeze.

1. Check Jenkins results:
   1. [ ] Check Jenkins jobs for failures: [distcheck](https://jenkins.aws.isc.org/job/kea-dev/job/distcheck/), etc...
   1. [ ] Check [Jenkins Tests Report](https://jenkins.aws.isc.org/job/kea-dev/job/jenkins-tests-report/).
   1. [ ] Check [tarball check report](https://jenkins.aws.isc.org/job/kea-dev/job/tarball-internal/Kea_20Build_20Checks/)
1. [ ] Check [Performance Test Results](https://jenkins.isc.org/job/kea-dev/job/performance/KeaPerformanceReport/) in Jenkins for drops in performance.
1. Check versioning, ask the development team if:
   - the library versions are being updated
   - `KEA_HOOKS_VERSION` is being updated
   - [ ] create an issue for that for developers in Gitlab
   - script: [./tools/bump-lib-versions.sh](https://gitlab.isc.org/isc-projects/kea/-/blob/master/tools/bump-lib-versions.sh) Kea-q.w.e Kea-a.b.c (where `a.b.c` is the version to be released and `q.w.e` is the version previous to that)
1. [ ] Look at the issue numbers in commit descriptions. Add to ChangeLog a mention about any change with visible impact that had not been mentioned already.
1. If any changes have been done to database schemas, then:
   1. [ ] Check that a previously released schema has not been changed.
   1. [ ] Check that the additions to `dhcpdb_create.*sql`, and nothing more nor less than what was added in this release, is present in a `upgrade_*_to_*.sh.in` script that should also have been added in this release.
1. Prepare Release Notes
   1. [ ] Create Release Notes on Kea GitLab wiki and notify @tomek about that. It should be created under "release notes" directory, like this one: https://gitlab.isc.org/isc-projects/kea/-/wikis/release%20notes/release-notes-2.1.0
   1. [ ] Finish release notes and conduct its review
1. [ ] Run [release-pkgs-upload-internal](https://jenkins.aws.isc.org/job/kea-dev/job/release-pkgs-upload-internal/) and [release-pkgs-check-internal](https://jenkins.aws.isc.org/job/kea-dev/job/release-pkgs-check-internal/) to test repositories for correctness.
   1. If a new Cloudsmith repository is used, then:
      1. [ ] Make sure freeradius packages are uploaded to the Cloudsmith repository or copied from a previous repository.
      1. [ ] Make sure access tokens have been been synchronized from previous Cloudsmith repositories and to the [check-pkgs.py](https://gitlab.isc.org/isc-private/qa-dhcp/-/blob/master/kea/pkgs-check/check-pkgs.py) QA tool.
1. [ ] Check if ReadTheDocs can build Kea documentation.
   1. Trigger rebuilding docs on [readthedocs.org](https://readthedocs.org/projects/kea/builds) and wait for the build to complete.

The following steps may involve changing files in the repository.

1. [ ] Run [update-code-for-release.py](https://gitlab.isc.org/isc-private/qa-dhcp/-/blob/master/kea/build/update-code-for-release.py) <br>
   Example command: `GITLAB_KEA_TOKEN='...' GITLAB_KEA_PREMIUM_TOKEN='...' ./update-code-for-release.py 1.9.7 'Apr 28, 2021' ~/isc/repos/kea/` <br>
   The script:
   - creates Gitlab issue and MR for release changes
   - adds release entries to ChangeLogs
   - regenerates BNF grammar
   - regenerates documentation
   - regenerates messages
   - reorders messages in alphabetical order
   - regenerates parsers
   - updates copyright dates
   - pushes the changes to MR
1. Check manually User's Guide sections:
   1. Chapter 1. Introduction
      1. [ ] On what platforms we are running tests using Jenkins? Update Supported Platforms in platforms.rst file.
      1. [ ] Did we add any additional 3rd party software? Update if needed
      1. [ ] Is there a new tool installed in bin or sbin released this time? If yes, is it documented?
   1. Chapter 2. Quick Start
      1. [ ] Has the default installation process changed (for kea and hooks)? If yes, are those changes documented and highlighted in the release notes?
   1. Chapter 3. Installation
      1. [ ] Check installation hierarchy (this is also automatically checked at the end of [ut-extended job](https://jenkins.aws.isc.org/job/kea-dev/job/ut-extended/))
      1. [ ] Check and update Build Requirements
      1. [ ] Check configure options against what `./configure -h` says
1. [ ] Check ChangeLog entries in Kea main and premium: spelling, trailing whitespaces, etc.
1. [ ] Check AUTHORS, INSTALL, README files in Kea main and premium.
   -  AUTHORS: update credits
   -  README: check "provides" with Release Notes, User Guide (1.3 Kea Software)
1. [ ] If changes were made, commit the change, push the branch to the main repository and request a review. Once the changes have been approved, merge the MR to master.

## Build selection, tarballs upload and sanity checks

This is the last moment to freeze code! :snowflake:

1. [ ] Go to [tarball-internal](https://jenkins.aws.isc.org/job/kea-dev/job/tarball-internal/) Jenkins job and pick the last tarball built - it will be a release candidate.
1. [ ] Check tarball before requesting sanity checks from the development team.
   1. Download tarballs from picked Jenkins build
   1. Check hook libraries.
      1. Are there any new hook libraries installed in this release?
         1. Are they in the proper tarball? Premium or subscription?
         1. Do they have their own package?
   1. Check sizes - is the new package reasonable?
   1. Check installation tree, compare it with the previous release
   1. Check installed libraries.
      1. which were updated? (save results)
      1. Do any of the libraries from the current release have lower version than in the previous release?
   1. Uninstall Kea, check what left (there should be just configuration files)
   1. Check if all of the installed binaries has man page
      1. if not, is it in the tarball?
      1. are man page up-to-date?
   1. Check if documentation is properly formatted, has correct versions and dates.
      1. it's advised to search for previous version numbers, some of them are statically added in statements that are no longer valid
1. [ ] Upload tarballs to repo.isc.org using Jenkins and send sanity checks request.
   1. Go to [release-tarball-upload-internal](https://jenkins.aws.isc.org/job/kea-dev/job/release-tarball-upload-internal/) Jenkins job.
   1. Click "Build with Parameters"
   1. In field "Tarball" select picked tarball build
   1. In field "Release_Candidate" pick:
      1. rc1 if this is the first selected build for release, it will push the selected tarballs to repo.isc.org, to a directory suffixed with indicated rc#
      1. next rc# if this is a respin after some fixes (note: it is not possible to pick previous rc number - it will result in an error)
      1. final if the last rc number was ok, this will push the selected tarball to repo.isc.org, to a directory with no suffixes
   1. Submit the job that will automatically:
      1. Upload the tarballs <br>
      and if this is not the final version:
      1. Create a GitLab issue for sanity checks, put there the announcement
      1. Send Sanity Checks announcement via email to dhcp-team@isc.org and to DHCP channel on Mattermost.<br>
      The announcement includes:
         - a link to chapter 4 Sanity Checks of the release process: [KeaReleaseProcess - SanityChecks](https://wiki.isc.org/bin/view/QA/KeaReleaseProcess#4.%20Sanity%20Checks)
         - a link to the GitLab issue
         - tarballs locations with SHA256 checksums
         - rpm/deb packages locations and versions


## Releasing Tarballs and Packages

1. [ ] Update Release Notes with ChangeLog entries
1. [ ] Upload final RPM & DEB packages to cloudsmith.io
   1. Go to [release-pkgs-upload-internal](https://jenkins.aws.isc.org/job/kea-dev/job/release-pkgs-upload-internal/).
   1. Click "Build with Parameters" link
   1. Pick your selected pkg build in Packages field, and select `PrivPubRepos: "both"`, `TestProdRepos: "production"` and click Build button.
   1. When it finishes run check: [releases-pkgs-check-internal](https://jenkins.aws.isc.org/job/kea-dev/job/release-pkgs-check-internal/).
1. [ ] Upload final tarballs to repo.isc.org
   1. Go to [release-tarball-upload-internal](https://jenkins.aws.isc.org/job/kea-dev/job/release-tarball-upload-internal/) Jenkins job.
   1. Click "Build with Parameters"
   1. In field "Tarball" select picked tarball build
   1. In field "Release_Candidate" pick final <br>
   This job will also:
      - open an issue on [the signing repository](https://gitlab.isc.org/isc-private/signing/-/issues) requesting signing final tarballs on repo.isc.org
      - create Git tags `Kea-a.b.c` in Kea main and premium repositories
      - send a signing request issue link on the DHCP Mattermost channel
1. [ ] Update ReadTheDocs
   1. Trigger rebuilding docs on [readthedocs.org](https://readthedocs.org/projects/kea/builds).
   1. Publish currently released version. On the `Versions` tab, scroll down to `Activate a version`, search for `kea-a.b.c` and click `Activate`.
   1. For stable releases, change the default version to point to this stable release.
1. [ ] Mark Jenkins jobs with release artifacts to be kept forever: <br>
   Go to the following Jenkins jobs, click release build and then, on the build page, click `Keep this build forever` button: <br>
   1. [tarball-internal job](https://jenkins.aws.isc.org/job/kea-dev/job/tarball-internal/)
   1. [pkg job](https://jenkins.aws.isc.org/job/kea-dev/job/pkg/)
1. [ ] Create an issue and a merge request to bump up Kea version in `configure.ac` to next development version which could be, based on just released version `a.b.c`:
    * `a.b.z-git` where `z == c + 1` or
    * `a.y.0-git` where `y == b + 1` or
    * `x.1.0-git` where `x == a + 1`
1. [ ] Send a request for publishing the release on the Support Mattermost channel linking the Signing issue and the release checklist issue.


### On the Day of Public Release

 - [ ] ***(Support)*** Wait for clearance from Security Officer to proceed with the public release (if applicable).
 - [ ] ***(Support)*** Wait for the signing ticket from the release engineer.
 - [ ] ***(Support)*** Confirm that the tarballs have the checksums mentioned on the signing ticket.
 - [ ] ***(Support)*** Sign the tarballs.
 - [ ] ***(Support)*** Upload signature files to repo.isc.org.
 - [ ] ***(Support)*** Place tarballs in public location on FTP site.
 - [ ] ***(Support)*** Publish links to downloads on ISC website.
 - [ ] ***(Support)*** Write release email to *kea-announce*.
 - [ ] ***(Support)*** Write email to *kea-users* (if a major release).
 - [ ] ***(Support)*** Send eligible customers updated links to the Subscription software FTP site.
 - [ ] ***(Support)*** If it is a new `major.minor` version, SWENG will have created a new repo in Cloudsmith, which will need the customer tokens migrated from an existing repo. Then update support customers that this new private repo exists.
 - [ ] ***(Support)*** Update tickets in case of waiting for support customers.
 - [ ] ***(QA)*** Inform Marketing of the release.
 - [ ] ***(Marketing)*** If a new Cloudsmith repository is used, update the Zapier scripts.
 - [ ] ***(Marketing)*** Upload Premium hooks tarball to SendOwl. Create a new product if a new branch, otherwise update existing product. Send notifications to existing subscribers of the new version.
 - [ ] ***(Marketing)*** Announce on social media.
 - [ ] ***(Marketing)*** Update [Wikipedia entry for Kea](https://en.wikipedia.org/wiki/Kea_(software)).
 - [ ] ***(Marketing)*** Write blog article (if a major release).
 - [ ] ***(Marketing)*** Update [Kea page on web site if any new hooks](https://www.isc.org/kea/).
 - [ ] ***(Marketing)*** Update Kea Premium and Kea Subscription data sheets if any new hooks.
 - [ ] ***(Marketing)*** Update [significant features matrix](https://kb.isc.org/docs/en/aa-01615) (if any significant new features).
 - [ ] ***(Marketing)*** Update [Kea documentation page in KB](https://kb.isc.org/docs/en/kea-administrator-reference-manual).
