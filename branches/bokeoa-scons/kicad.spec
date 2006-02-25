Name:		kicad
Version:	20060119
Release:	1%{?dist}
Summary:	Electronic PCB design and schematic creation with 3D preview
URL:		http://www.lis.inpg.fr/realise_au_lis/kicad/
License:	GPL
Group:		Applications/Engineering
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires:	wxGTK-devel >= 2.6.1 scons
Source0:	kicad-sources-2006-01-19.tar.bz2
Source1:	kicad-data-2006-01-19.tar.bz2


%description
KiCad is an open source (GPL) software for the creation of electronic
schematic diagrams and printed circuit board artwork.

KiCad is a set of four softwares and a project manager:

   eeschema - schematic entry
   pcbnew   - PCB board editor
   gerbview - GERBER viewer (photoplotter documents)
   cvpcb    - footprint selector for components used in the circuit design
   kicad    - project manager


%prep
%setup -q -n kicad-sources-2006-01-19


%build
scons %{?_smp_mflags}


%install
rm -rf %{buildroot}

mkdir -p %{buildroot}%{_bindir}
install -m 0755 cvpcb/cvpcb %{buildroot}%{_bindir}/cvpcb
install -m 0755 eeschema/eeschema %{buildroot}%{_bindir}/eeschema
install -m 0755 gerbview/gerbview %{buildroot}%{_bindir}/gerbview
install -m 0755 kicad/kicad %{buildroot}%{_bindir}/kicad
install -m 0755 pcbnew/pcbnew %{buildroot}%{_bindir}/pcbnew

mkdir -p %{buildroot}%{_docdir}
mkdir -p %{buildroot}%{_datadir}/%{name}
(cd %{buildroot}%{_docdir}
   tar jxf %SOURCE1
   mv kicad-data-2006-01-19 kicad
   mv kicad/{internat,library,modules,template} %{buildroot}%{_datadir}/%{name}
)

%clean
rm -rf %{buildroot}


%files
%defattr(-,root,root)
%{_bindir}/*
%{_docdir}/%{name}/
%{_datadir}/%{name}/


%changelog
* Fri Feb 24 2006 Bryan Stillwell <bryan@bokeoa.com> - 20060119-1
- Updated for the 2006-01-19 release

* Fri Nov 18 2005 Bryan Stillwell <bryan@bokeoa.com> - 20051027-0.01
- Initial version
