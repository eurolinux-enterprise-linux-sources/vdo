%global commit       3ec8918b7c60cecd7ca3c2e7d74e0b38fb341e39
%global gittag       6.1.2.41
%global shortcommit  %(c=%{commit}; echo ${c:0:7})
%define spec_release 4
#
#
#
Summary: Management tools for Virtual Data Optimizer
Name: vdo
Version: %{gittag}
Release: %{spec_release}%{?dist}
License: GPLv2
Source0: https://github.com/dm-vdo/%{name}/archive/%{commit}/%{name}-%{shortcommit}.tar.gz
URL: http://github.com/dm-vdo/vdo
Distribution: RHEL 7.7
Requires: PyYAML >= 3.10
Requires: libuuid >= 2.23
Requires: kmod-kvdo >= 6.1
Requires: lvm2 >= 2.02.171
Provides: kvdo-kmod-common = %{version}
ExclusiveArch: x86_64
ExcludeArch: s390
ExcludeArch: s390x
ExcludeArch: ppc
ExcludeArch: ppc64
ExcludeArch: ppc64le
ExcludeArch: aarch64
ExcludeArch: i686
BuildRequires: gcc
BuildRequires: device-mapper-event-devel
BuildRequires: libuuid-devel
BuildRequires: python
BuildRequires: python-devel
BuildRequires: systemd
BuildRequires: valgrind-devel
BuildRequires: zlib-devel
%{?systemd_requires}

# Disable an automatic dependency due to a file in examples/monitor
%define __requires_exclude perl

%description
Virtual Data Optimizer (VDO) is a device mapper target that delivers
block-level deduplication, compression, and thin provisioning.

This package provides the user-space management tools for VDO.

%prep
%setup -n %{name}-%{commit}

%build
make

%install
make install DESTDIR=$RPM_BUILD_ROOT INSTALLOWNER= bindir=%{_bindir} \
  defaultdocdir=%{_defaultdocdir} name=%{name} \
  python_sitelib=%{python_sitelib} mandir=%{_mandir} \
  unitdir=%{_unitdir} presetdir=%{_presetdir}

%post
%systemd_post vdo.service

%preun
%systemd_preun vdo.service

%postun
%systemd_postun_with_restart vdo.service

%files
#defattr(-,root,root)
%{_bindir}/vdo
%{_bindir}/vdodmeventd
%{_bindir}/vdodumpconfig
%{_bindir}/vdoforcerebuild
%{_bindir}/vdoformat
%{_bindir}/vdoprepareupgrade
%{_bindir}/vdoreadonly
%{_bindir}/vdostats
%dir %{python_sitelib}/%{name}
%dir %{python_sitelib}/%{name}/vdomgmnt/
%{python_sitelib}/%{name}/vdomgmnt/CommandLock.py
%{python_sitelib}/%{name}/vdomgmnt/CommandLock.pyc
%{python_sitelib}/%{name}/vdomgmnt/CommandLock.pyo
%{python_sitelib}/%{name}/vdomgmnt/Configuration.py
%{python_sitelib}/%{name}/vdomgmnt/Configuration.pyc
%{python_sitelib}/%{name}/vdomgmnt/Configuration.pyo
%{python_sitelib}/%{name}/vdomgmnt/Constants.py
%{python_sitelib}/%{name}/vdomgmnt/Constants.pyc
%{python_sitelib}/%{name}/vdomgmnt/Constants.pyo
%{python_sitelib}/%{name}/vdomgmnt/Defaults.py
%{python_sitelib}/%{name}/vdomgmnt/Defaults.pyc
%{python_sitelib}/%{name}/vdomgmnt/Defaults.pyo
%{python_sitelib}/%{name}/vdomgmnt/ExitStatusMixins.py
%{python_sitelib}/%{name}/vdomgmnt/ExitStatusMixins.pyc
%{python_sitelib}/%{name}/vdomgmnt/ExitStatusMixins.pyo
%{python_sitelib}/%{name}/vdomgmnt/KernelModuleService.py
%{python_sitelib}/%{name}/vdomgmnt/KernelModuleService.pyc
%{python_sitelib}/%{name}/vdomgmnt/KernelModuleService.pyo
%{python_sitelib}/%{name}/vdomgmnt/MgmntLogger.py
%{python_sitelib}/%{name}/vdomgmnt/MgmntLogger.pyc
%{python_sitelib}/%{name}/vdomgmnt/MgmntLogger.pyo
%{python_sitelib}/%{name}/vdomgmnt/MgmntUtils.py
%{python_sitelib}/%{name}/vdomgmnt/MgmntUtils.pyc
%{python_sitelib}/%{name}/vdomgmnt/MgmntUtils.pyo
%{python_sitelib}/%{name}/vdomgmnt/Service.py
%{python_sitelib}/%{name}/vdomgmnt/Service.pyc
%{python_sitelib}/%{name}/vdomgmnt/Service.pyo
%{python_sitelib}/%{name}/vdomgmnt/SizeString.py
%{python_sitelib}/%{name}/vdomgmnt/SizeString.pyc
%{python_sitelib}/%{name}/vdomgmnt/SizeString.pyo
%{python_sitelib}/%{name}/vdomgmnt/Utils.py
%{python_sitelib}/%{name}/vdomgmnt/Utils.pyc
%{python_sitelib}/%{name}/vdomgmnt/Utils.pyo
%{python_sitelib}/%{name}/vdomgmnt/VDOService.py
%{python_sitelib}/%{name}/vdomgmnt/VDOService.pyc
%{python_sitelib}/%{name}/vdomgmnt/VDOService.pyo
%{python_sitelib}/%{name}/vdomgmnt/VDOKernelModuleService.py
%{python_sitelib}/%{name}/vdomgmnt/VDOKernelModuleService.pyc
%{python_sitelib}/%{name}/vdomgmnt/VDOKernelModuleService.pyo
%{python_sitelib}/%{name}/vdomgmnt/VDOOperation.py
%{python_sitelib}/%{name}/vdomgmnt/VDOOperation.pyc
%{python_sitelib}/%{name}/vdomgmnt/VDOOperation.pyo
%{python_sitelib}/%{name}/vdomgmnt/__init__.py
%{python_sitelib}/%{name}/vdomgmnt/__init__.pyc
%{python_sitelib}/%{name}/vdomgmnt/__init__.pyo
%dir %{python_sitelib}/%{name}/statistics/
%{python_sitelib}/%{name}/statistics/Command.py
%{python_sitelib}/%{name}/statistics/Command.pyc
%{python_sitelib}/%{name}/statistics/Command.pyo
%{python_sitelib}/%{name}/statistics/Field.py
%{python_sitelib}/%{name}/statistics/Field.pyc
%{python_sitelib}/%{name}/statistics/Field.pyo
%{python_sitelib}/%{name}/statistics/KernelStatistics.py
%{python_sitelib}/%{name}/statistics/KernelStatistics.pyc
%{python_sitelib}/%{name}/statistics/KernelStatistics.pyo
%{python_sitelib}/%{name}/statistics/LabeledValue.py
%{python_sitelib}/%{name}/statistics/LabeledValue.pyc
%{python_sitelib}/%{name}/statistics/LabeledValue.pyo
%{python_sitelib}/%{name}/statistics/StatFormatter.py
%{python_sitelib}/%{name}/statistics/StatFormatter.pyc
%{python_sitelib}/%{name}/statistics/StatFormatter.pyo
%{python_sitelib}/%{name}/statistics/StatStruct.py
%{python_sitelib}/%{name}/statistics/StatStruct.pyc
%{python_sitelib}/%{name}/statistics/StatStruct.pyo
%{python_sitelib}/%{name}/statistics/VDOReleaseVersions.py
%{python_sitelib}/%{name}/statistics/VDOReleaseVersions.pyc
%{python_sitelib}/%{name}/statistics/VDOReleaseVersions.pyo
%{python_sitelib}/%{name}/statistics/VDOStatistics.py
%{python_sitelib}/%{name}/statistics/VDOStatistics.pyc
%{python_sitelib}/%{name}/statistics/VDOStatistics.pyo
%{python_sitelib}/%{name}/statistics/__init__.py
%{python_sitelib}/%{name}/statistics/__init__.pyc
%{python_sitelib}/%{name}/statistics/__init__.pyo
%dir %{python_sitelib}/%{name}/utils/
%{python_sitelib}/%{name}/utils/Command.py
%{python_sitelib}/%{name}/utils/Command.pyc
%{python_sitelib}/%{name}/utils/Command.pyo
%{python_sitelib}/%{name}/utils/FileUtils.py
%{python_sitelib}/%{name}/utils/FileUtils.pyc
%{python_sitelib}/%{name}/utils/FileUtils.pyo
%{python_sitelib}/%{name}/utils/Logger.py
%{python_sitelib}/%{name}/utils/Logger.pyc
%{python_sitelib}/%{name}/utils/Logger.pyo
%{python_sitelib}/%{name}/utils/Timeout.py
%{python_sitelib}/%{name}/utils/Timeout.pyc
%{python_sitelib}/%{name}/utils/Timeout.pyo
%{python_sitelib}/%{name}/utils/Transaction.py
%{python_sitelib}/%{name}/utils/Transaction.pyc
%{python_sitelib}/%{name}/utils/Transaction.pyo
%{python_sitelib}/%{name}/utils/YAMLObject.py
%{python_sitelib}/%{name}/utils/YAMLObject.pyc
%{python_sitelib}/%{name}/utils/YAMLObject.pyo
%{python_sitelib}/%{name}/utils/__init__.py
%{python_sitelib}/%{name}/utils/__init__.pyc
%{python_sitelib}/%{name}/utils/__init__.pyo
%{_unitdir}/vdo.service
%{_presetdir}/97-vdo.preset
%dir %{_defaultdocdir}/%{name}
%license %{_defaultdocdir}/%{name}/COPYING
%dir %{_defaultdocdir}/%{name}/examples
%dir %{_defaultdocdir}/%{name}/examples/ansible
%doc %{_defaultdocdir}/%{name}/examples/ansible/README.txt
%doc %{_defaultdocdir}/%{name}/examples/ansible/test_vdocreate.yml
%doc %{_defaultdocdir}/%{name}/examples/ansible/test_vdocreate_alloptions.yml
%doc %{_defaultdocdir}/%{name}/examples/ansible/test_vdoremove.yml
%dir %{_defaultdocdir}/%{name}/examples/monitor
%doc %{_defaultdocdir}/%{name}/examples/monitor/monitor_check_vdostats_logicalSpace.pl
%doc %{_defaultdocdir}/%{name}/examples/monitor/monitor_check_vdostats_physicalSpace.pl
%doc %{_defaultdocdir}/%{name}/examples/monitor/monitor_check_vdostats_savingPercent.pl
%dir %{_defaultdocdir}/%{name}/examples/systemd
%doc %{_defaultdocdir}/%{name}/examples/systemd/VDO.mount.example
%{_mandir}/man8/vdo.8.gz
%{_mandir}/man8/vdodmeventd.8.gz
%{_mandir}/man8/vdodumpconfig.8.gz
%{_mandir}/man8/vdoforcerebuild.8.gz
%{_mandir}/man8/vdoformat.8.gz
%{_mandir}/man8/vdostats.8.gz

%changelog
* Wed Mar 27 2019 - Andy Walsh <awalsh@redhat.com> - 6.1.2.41-4
- Removed the VDO Ansible module and associated examples from this package.
  - Resolves: rhbz#1536515
- Improved error messages from the vdo script.
  - Resolves: rhbz#1688481

* Tue Mar 19 2019 - Andy Walsh <awalsh@redhat.com> - 6.1.2.38-3
- Fixed more error path memory leaks.
  - Resolves: rhbz#1609426
- Improved man pages.
  - Resolves: rhbz#1636047
- Improved checking for existing LVM physical volumes when formatting a new
  VDO volume.
  - Resolves: rhbz#1687797
- Fixed a bug which would cause the default logical size to be 480 KB
  smaller than it could be without causing over-provisioning.
  - Resolves: rhbz#1532697
- Removed extraneous man pages.
  - Resolves: rhbz#1643284
- Allowed VDO backing devices to be specified by major:minor device number.
  - Resolves: rhbz#1637762
- Improved behavior when attempting to create a VDO volume whose name conflicts
  with an existing dm target.
  - Resolves: rhbz#1588140
- Modified vdo script to report fewer errors when removing a VDO which
  sits atop a missing device.
  - Resolves: rhbz#1535476

* Fri Sep 14 2018 - Andy Walsh <awalsh@redhat.com> - 6.1.1.125-3
- Re-sync with the 'kmod-kvdo' package.
- Related: rhbz#1628318

* Sun Jul 29 2018 - Andy Walsh <awalsh@redhat.com> - 6.1.1.120-3
- Improved error messages in the vdo script when failing to stop a device.
- Resolves: rhbz#1511106
- Improved man pages.
- Resolves: rhbz#1572640
- Modified vdo script to ignore (but preserve) unrecognised parameters in
  the vdo config file so that config files are compatible across versions.
- Resolves: rhbz#1604060
- Updated Ansible module to change block map cache size on existing volumes
- Resolves: rhbz#1541170

* Sun Jul 15 2018 - Andy Walsh <awalsh@redhat.com> - 6.1.1.111-3
- Added support for issuing fullness warnings via dmeventd
- rhbz#1519307
- Provided missing error code to text translation for vdoDumpConfig.
- Resolves: rhbz#1595129
- Improved error reporting when using pvcreate in the vdo script to check
  for pre-existing devices when creating a VDO volume.
- Resolves: rhbz#1557464

* Wed Jun 20 2018 - Andy Walsh <awalsh@redhat.com> - 6.1.1.99-2
- Modified the vdo script to not allow creation of a VDO device on top of
  an already running VDO device.
- Resolves: rhbz#1572640
- Made the ordering of the output of vdo list stable.
- Resolves: rhbz#1589363
- Added the proper dist tag
- Resolves: rhbz#1588043

* Fri May 11 2018 - Andy Walsh <awalsh@redhat.com> - 6.1.1.84-1
- Improved and corrected man pages.
- Improved error handling in vdo script when stopping volumes in use.
- Resolves: rhbz#1511106
- Modified the vdo script to return more specific error exit codes.
- Resolves: rhbz#1543954
- Improved detection of existing formats when creating new volumes.
- Resolves: rhbz#1557464
- Improved VDO Ansible testing playbooks.
- Fixed compilation errors on newer versions of GCC.
- Removed the --physical-size option to vdoformat since VDO now always uses
  the entire device on which it resides.
- Modified vdo script to use /dev/disk/by-id to describe the location of
  VDO volumes when possible.
- Fixed a bug where VDO would always be created with a dense index even
  when a sparse index was requested.
- Resolves: rhbz#1570156
- Modified the vdo script to accept decimal points or commas when
  specifying the --indexMem option.
- Resolves: rhbz#1552146
- Changed vdo script to not accept --vdoSlabSize=0 as a way of specifying
  the default since it was confusing. The default can be obtained by merely
  omitting the parameter entirely.
- Resolves: rhbz#1564181
- Modified the vdo script to require the --force option to remove a VDO
  whose underlying device is not present.
- Resolves: rhbz#1535476

* Thu May 10 2018 - Andy Walsh <awalsh@redhat.com> - 6.1.1.24-1
- Rebased to 6.1.1 branch from github
- Resolves: rhbz#1576701
- Removed obsolete nagios module.
- Modified spec files to use systemd macros
- Resolves: rhbz#1531111
- Updated the vdo.8 man page
- Resolves: rhbz#1547786
- Improved some error messages

* Fri Feb 16 2018 - Joseph Chapman <jochapma@redhat.com> - 6.1.0.149-16
- Sync mode is safe if underlying storage changes to requiring flushes
- Resolves: rhbz#1540777

* Wed Feb 07 2018 - Joseph Chapman <jochapma@redhat.com> - 6.1.0.146-16
- VDO start command now does index memory checking
- Module target is now "vdo" instead of "dedupe"
- VDO remove with no device no longer puts a spurious file in /dev
- ANsible module no longer fails on modification operations
- Resolves: rhbz#1510567
- Resolves: rhbz#1530358
- Resolves: rhbz#1535597
- Resolves: rhbz#1536214

* Tue Feb 06 2018 - Andy Walsh <awalsh@redhat.com> - 6.1.0.144-16
- Updated summary and description
- Resolves: rhbz#1541409

* Thu Feb 01 2018 - Joseph Chapman <jochapma@redhat.com> - 6.1.0.130-15
- vdo growLogical by less than 4K gives correct error
- Fix URL to point to GitHub tree
- Resolves: rhbz#1532653
- Resolves: rhbz#1539059

* Fri Jan 19 2018 - Joseph Chapman <jochapma@redhat.com> - 6.1.0.124-14
- Added a specific error for less than 1 block growLogical.
- Resolves: rhbz#1532653

* Wed Jan 10 2018 - Joseph Chapman <jochapma@redhat.com> - 6.1.0.114-14
- VDO automatically chooses the proper write policy by default
- Package uninstall removes vdo.service symlinks
- Resolves: rhbz#1525305
- Resolves: rhbz#1531047

* Thu Dec 21 2017 - Joseph Chapman <jochapma@redhat.com> - 6.1.0.106-13
- Handle bogus --confFile and --logfile arguments
- Produce more informative vdo manager high-level help
- Generate command-specific unrecognized argument messages
- Resolves: rhbz#1520927
- Resolves: rhbz#1522750
- Resolves: rhbz#1525560

* Tue Dec 12 2017 - Joseph Chapman <jochapma@redhat.com> - 6.1.0.97-13
- Remove vdo --noRun option
- Clean up vdo error handling
- Prevent python stack traces on vdo errors
- Add more bounds checking to indexMem
- Resolves: rhbz#1508544
- Resolves: rhbz#1508918
- Resolves: rhbz#1520991
- Resolves: rhbz#1522754

* Fri Dec 08 2017 - Joseph Chapman <jochapma@redhat.com> - 6.1.0.89-13
- Build changes for UUID
- Limit VDO physical size
- Limit command options to those applicable to the subcommand
- Fix vdo --modifyBlockMapPeriod
- Report missing command option appropriately for all subcommands
- Fix behavior of --indexMem when there's not enough memory
- Remove obsolete nagios plugin from examples
- Better error behavior for failing vdo status commands
- Fix boundary check error for vdoLogicalSize
- Resolves: rhbz#1507927
- Resolves: rhbz#1508452
- Resolves: rhbz#1508544
- Resolves: rhbz#1508918
- Resolves: rhbz#1509002
- Resolves: rhbz#1510567
- Resolves: rhbz#1512631
- Resolves: rhbz#1522943

* Fri Dec 01 2017 - Joseph Chapman <jochapma@redhat.com> - 6.1.0.72-12
- Don't corrupt an existing filesystem with "vdo create" without "--force"
- Resolves: rhbz#1510581

* Mon Nov 27 2017 - Ken Raeburn <raeburn@redhat.com> - 6.1.0.55-11
- Don't corrupt an existing filesystem with "vdo create" without "--force"
- Resolves: rhbz#1510581

* Fri Nov 17 2017 - Joseph Chapman <jochapma@redhat.com> - 6.1.0.55-10
- manual pages: note logical size limit of 4P
- manual pages: make cache size/thread count link clearer
- Resolves: rhbz#1508452
- Resolves: rhbz#1511042

* Fri Nov 03 2017 - Joseph Chapman <jochapma@redhat.com> - 6.1.0.46-9
- update manpage to not allow 0 as an option
- enforce maximum vdoPhysicalThreads
- update manpage to describe maximum vdoPhysicalThreads
- Resolves: rhbz#1510405
- Resolves: rhbz#1511075
- Resolves: rhbz#1511085
- Resolves: rhbz#1511091

* Fri Nov 03 2017 - Joseph Chapman <jochapma@redhat.com> - 6.1.0.34-8
- Bugfixes
- Resolves: rhbz#1480047

* Mon Oct 30 2017 - Joseph Chapman <jochapma@redhat.com> - 6.1.0.0-7
- Don't let make install try to set file ownerships itself
- Resolves: rhbz#1480047

* Thu Oct 12 2017 - Joseph Chapman <jochapma@redhat.com> - 6.1.0.0-6
- Added new man pages
- Resolves: rhbz#1480047

* Fri Oct  6 2017 - Joseph Chapman <jochapma@redhat.com> - 6.1.0.0-5
- Fixed a typo in the package description
- Fixed man page paths
- Resolves: rhbz#1480047

* Thu Oct  5 2017 - Joseph Chapman <jochapma@redhat.com> - 6.1.0.0-4
- Fix vdostats name in nagios examples
- Build only on the x86_64 architecture
- Add systemd files
- Resolves: rhbz#1480047

* Thu Oct  5 2017 - Joseph Chapman <jochapma@redhat.com> - 6.1.0.0-3
- Added missing Build-Requires and incorporated naming changes
- Resolves: rhbz#1480047

* Wed Oct  4 2017 - Joseph Chapman <jochapma@redhat.com> - 6.1.0.0-2
- Fixed requirements and tags in %files section
- Resolves: rhbz#1480047

* Tue Oct  3 2017 - Joseph Chapman <jochapma@redhat.com> - 6.1.0.0-1
- Initial implementation
- Resolves: rhbz#1480047
