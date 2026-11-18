from unittest import TestCase

from app import greet


class GreetTest(TestCase):
    def test_greet(self):
        self.assertEqual(greet("orca"), "hello orca, from WSL")
